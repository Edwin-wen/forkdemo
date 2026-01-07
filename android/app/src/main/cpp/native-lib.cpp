#include <jni.h>
#include <string>
#include <unistd.h>
#include <android/log.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <fstream>
#include <sstream>
#include <time.h>
#include <dlfcn.h>

#define TAG "ForkDemo"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

struct TestData
{
    int value = 10;
};

class TestClass
{
public:
    int value = 20;
};

// 线程参数结构体
struct ThreadParams
{
    int value1;
    int value2;
    int result;
    pid_t parent_pid;
    std::string hello_message;  // 保存 hello() 返回的字符串
};

// 信号处理函数
void signal_handler(int signo)
{
    if (signo == SIGUSR1)
    {
        LOGI("父进程收到子进程信号，计算完成，结果已写入文件 /sdcard/Download/fork_result.txt");
    }
}

// 线程函数
void* thread_func(void* arg)
{
    ThreadParams* params = (ThreadParams*)arg;
    
    // 计算两个value相加
    params->result = params->value1 + params->value2;
    
    // 在/sdcard/Download目录创建文件
    std::string file_path = "/sdcard/Download/fork_result.txt";
    std::ofstream file(file_path.c_str(), std::ios::out | std::ios::trunc);
    
    if (file.is_open())
    {
        // 获取当前时间
        time_t now = time(nullptr);
        char time_str[64];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
        
        // 写入文件内容
        file << "Fork子进程计算结果\n";
        file << "时间: " << time_str << "\n";
        file << "计算: " << params->value1 << " + " << params->value2 << " = " << params->result << "\n";
        file << "子进程PID: " << getpid() << "\n";
        file << "父进程PID: " << params->parent_pid << "\n";
        if (!params->hello_message.empty())
        {
            file << "libaddlib.so hello() 返回: " << params->hello_message << "\n";
        }
        
        file.close();
    }
    
    // 通知父进程
    kill(params->parent_pid, SIGUSR1);
    
    return nullptr;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_edwin_forkdemo_MainActivity_testfork(
        JNIEnv* env,
        jobject /* this */) {
    TestData data;
    TestClass* testClass = new TestClass();
    jint pid = fork();
    if (pid == 0)
    {
        while(true)
		{
			// 调试子进程时在进程同级目录下加此文件
			if (access("/sdcard/Download/fork.debug", F_OK) == -1)
				break;
			else
				usleep(1000);
		}
        // 子进程
        data.value = 50;
        testClass->value = 50;
        
        // 用于保存 hello() 返回的字符串
        std::string hello_message = "";
        
        // 加载 libaddlib.so 动态库
        void* handle = dlopen("libaddlib.so", RTLD_LAZY);
        if (handle != nullptr)
        {
            LOGI("子进程: 成功加载 libaddlib.so");
            
            // 获取 hello 函数指针 (返回 const char*)
            typedef const char* (*hello_func)();
            hello_func hello = (hello_func)dlsym(handle, "hello");
            
            if (hello != nullptr)
            {
                LOGI("子进程: 找到 hello 函数,准备调用");
                // 调用 hello 函数并获取返回值
                const char* result = hello();
                if (result != nullptr)
                {
                    hello_message = result;
                    LOGI("子进程: hello 函数返回: %s", result);
                }
                else
                {
                    LOGI("子进程: hello 函数返回 nullptr");
                }
                LOGI("子进程: hello 函数调用完成");
            }
            else
            {
                LOGI("子进程: 无法找到 hello 函数: %s", dlerror());
            }
            
            // 关闭动态库
            dlclose(handle);
        }
        else
        {
            LOGI("子进程: 无法加载 libaddlib.so: %s", dlerror());
        }
        
        // 准备线程参数
        ThreadParams params;
        params.value1 = data.value;
        params.value2 = testClass->value;
        params.result = 0;
        params.parent_pid = getppid();  // 获取父进程ID
        params.hello_message = hello_message;  // 传递 hello() 返回的字符串
        
        // 创建线程
        pthread_t thread_id;
        int ret = pthread_create(&thread_id, nullptr, thread_func, &params);
        if (ret == 0)
        {
            // 等待线程完成
            pthread_join(thread_id, nullptr);
        }
        
        // 子进程退出
        _exit(0);
    }
    else
    {
        // 父进程
        // 注册信号处理函数
        signal(SIGUSR1, signal_handler);
        
        LOGI("父进程：等待子进程信号...");
        // 等待子进程结束
        sleep(2);  // 给子进程足够的时间完成
    }
    return pid;
}