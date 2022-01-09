# C0编译器

查阅PDF找到更多设计细节

# 测试案例

1. 功能测试：  
test_F1.txt  
test_F2.txt  
test_F3.txt  
test_F4.txt  
test_F5.txt  
test_F6.txt  
test_F7.txt  
test_F8.txt  
test_F9.txt  
test_F10.txt  

2. 性能及综合测试：  
test_S1.txt  
test_S2.txt
test_S3.txt

3. 错误处理测试：  
test_E1.txt

4. 优化测试：  
test_O1.txt

# 词法分析

1.读入程序  
2.检查程序  
3.判断是否出错  

# 语法分析

# 中间代码生成

# 中间代码优化
1. 窥孔优化
2. 消除公共子表达式

# 汇编语言生成

# 寄存器分配优化
$0 保持常0  
$1 保留给MARS使用  
$a0 用作函数传参  
$v0 用作syscall参数或函数返回值  
  
$t0 用作临时操作变量  
$t1 用作保存四元式第一个操作数或四元式运算结果  
$t2 用作保存四元式第二个操作数  
$t3仅在数组赋值时保存数组基地址   
$t4 switch变量
  
$t5 临时分配

$sp 用作栈指针  
$fp 用作函数运行栈基地址  
$ra 用作返回地址  
