import { hilog } from '@kit.PerformanceAnalysisKit';
import testNapi from 'libentry.so';
import { process } from '@kit.ArkTS';

const DOMAIN = 0x0000;

@Entry
@Component
struct Index {
    @State message: string = 'Hello World';

    build() {
        Row() {
            Column() {
                Text(this.message)
                    .fontSize($r('app.float.page_text_font_size'))
                    .fontWeight(FontWeight.Bold)
                    .onClick(() => {
                        this.message = 'Welcome';
                        let pid = testNapi.testFork()
                        if (pid == 0) {
                            hilog.info(DOMAIN, 'arkFork', '子进程：%{public}d', process.pid);
                        } else {
                            hilog.info(DOMAIN, 'arkFork', '父进程：%{public}d', process.pid);
                        }
                    })
            }
            .width('100%')
        }
        .height('100%')
    }
}
