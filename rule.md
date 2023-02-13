# 0.1
Sample
```
int abs(int value)
{
    if (value < 0)
    {
        return -value;
    }
    return value;
}

int i = 114;
print(abs(i - 514));    
```
L_BNF
```
# 起始
语句    ->  函数定义
        |  IF-分支 
        |  变量定义语句
        |  变量赋值定义语句
        |  RETURN-语句
        |  赋值语句
        |  函数调用语句

函数定义    ->  identifier identifier ( 参数 ) 块

参数声明    -> identifier identifier
            | 参数声明 , identifier identifier

块  ->  { 语句集合 }

语句集合    ->  语句
            |  语句集合 语句

IF-分支 ->  if ( 值 ) 块

值  ->  integer
    |   identifier
    |   函数调用

函数调用    ->  identifier identifier ( 参数 )

参数    ->  值
        ->  参数 , 值

变量定义语句    ->  identifier identifier ;

变量赋值定义语句    ->  identifier identifier = 值 ;

RETURN-语句 ->  return 值 ;

赋值语句    ->  identifier = 值 ;

函数调用语句    ->  函数调用 ;

```