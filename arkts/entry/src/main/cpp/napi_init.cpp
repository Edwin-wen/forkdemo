#include "napi/native_api.h"
#include "unistd.h"
#include <hilog/log.h>

struct TestData 
{
	int value = 10;
};

class TestClass
{
public:
	int value = 20;
};

static napi_value Add(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};

    napi_get_cb_info(env, info, &argc, args , nullptr, nullptr);

    napi_valuetype valuetype0;
    napi_typeof(env, args[0], &valuetype0);

    napi_valuetype valuetype1;
    napi_typeof(env, args[1], &valuetype1);

    double value0;
    napi_get_value_double(env, args[0], &value0);

    double value1;
    napi_get_value_double(env, args[1], &value1);

    napi_value sum; 
    napi_create_double(env, value0 + value1, &sum);
    
    napi_value ret;
    pid_t pid = fork();
    napi_create_int32(env, pid, &ret);
    
    return ret;

}

static napi_value testFork(napi_env env, napi_callback_info info)
{
    napi_value ret;
	TestData data;
	TestClass* testClass = new TestClass();
    pid_t pid = fork();
    if (pid == 0)
	{
		OH_LOG_Print(LOG_APP, LOG_DEBUG, 0xFF00, "ForkTag", "子进程:%{public}d data:%{public}d class:%{public}d", getpid(), data.value, testClass->value);
	}
	else 
	{
		data.value = 20;
		testClass->value = 30;
		OH_LOG_Print(LOG_APP, LOG_DEBUG, 0xFF00, "ForkTag", "父进程:%{public}d data:%{public}d class:%{public}d", getpid(), data.value, testClass->value);
	}
    napi_create_int32(env, pid, &ret);
    return ret;

}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        { "add", nullptr, Add, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "testFork", nullptr, testFork, nullptr, nullptr, nullptr, napi_default, nullptr }
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "entry",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterEntryModule(void)
{
    napi_module_register(&demoModule);
}
