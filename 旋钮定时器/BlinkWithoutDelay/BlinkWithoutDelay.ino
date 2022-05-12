/* LED闪烁进阶

 让13引脚连接的LED闪烁起来而不使用delay()函数。这样就意味着其他的代码可以不受LED闪烁的干扰，在“同一时间”(译者注：其实应该是几乎同一时间)运行。

电路这样搭:
 * LED连接到13引脚和GND。
 * 注：绝大多数Arduino已经在13引脚连接了板载LED。因而这个例子可能不需要多余LED也能看到效果。

代码是公开的。

下列代码运用了mills()函数。mills()函数返回一个时间值，这个值表示从程序的运行时间(从通电或复位开始)，单位为毫秒（milliseconds ）。

有时候你需要同时处理多件事情。比如：在按钮按下的时候让LED闪烁。
在这种情况下delay()函数就不好用了，因为Arduino在delay的时候会完全暂停运行的程序。如果按钮按下而Arduino正在delay，那么你的程序就无法觉察到按钮被按下了。
本例程向你展示如何不用delay()而让LED闪烁。具体方法是：把LED打开后记录一下当前时间。接着，每次loop()函数被调用，就检查一下是否距离上一次记录的时间已经过了
想让LED点亮或关闭的时间。如果时间差足够，那么就将LED关闭或点亮，并且记录现在的时间……如此往复，LED就会照常闪烁，并且程序再也不会被LED的闪烁这样原先需要用delay函数的操作拖后腿了。

 */

// 定义一个不会改变的整型常量。这里用来定义引脚号码:
const int ledPin = 13; // LED连接的引脚

// 声明并定义可变的变量 :
int ledState = LOW; // LED的状态值

// 一般来说，用 "unsigned long"类型的变量来存储时间值比较好。因为如果用int类型“装不下”这么大的数字。
unsigned long previousMillis = 0; // 存储上次LED状态被改变的时间

// 又定义了一个常量 :
const long interval = 1000; // LED状态应该被改变的间隔时间(单位毫秒)

void setup()
{
    // 将数字引脚定义为输出模式：
    pinMode(ledPin, OUTPUT);
}

void loop()
{
    //这里写你想要不断运行的代码。

    // 检查看看LED是否到了应该打开或关闭的时间; 就是说，检查下现在时间离开记录的时间是否超过了要求LED状态改变的间隔时间。
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval)
    {
        // 更新时间标记
        previousMillis = currentMillis;

        // 如果LED关闭则打开它，如果LED打开则关闭它:
        if (ledState == LOW)
        {
            ledState = HIGH;
        }
        else
        {
            ledState = LOW;
        }

        // 用以下代码设置LED状态:
        digitalWrite(ledPin, ledState);
    }
}