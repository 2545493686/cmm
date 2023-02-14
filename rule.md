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
语句(STATEMENT) 
        |*  IF-分支       # if 
        |*  RETURN-语句      # return
        |  函数定义    # id id ( id id
        |*  变量定义语句     # id id ;
        |  变量赋值定义语句 # id id =
        |*  值语句           # value ;

函数定义    ->  id id ( 参数 ) 块

参数声明    -> id id
            | 参数声明 , id id

块  ->  { 语句集合 }

语句集合    ->  语句
            |  语句集合 语句

IF-分支(IF) ->*  if ( 值 ) 块

值(VALUE)   ->*  间接式

# 左递归的表达式
间接式(IndirectExpr)   ->*  赋值式

赋值式(Assign)  ->*  比较式 = 赋值式
                |*   比较式

比较式(Compare) ->*  加减式 <|> 加减式
                |*  加减式

加减式(AddSub)    ->*  乘除式 +|- 加减式
                    |*  乘除式

乘除式(MulDiv)   ->*  直接式 *|/ 乘除式 
                    ->*  直接式

# 非左递归的表达式
直接式(DirectExpr) ->*  integer    
                    |*   ( 值 )
                    |*   - 值
                    |*   函数调用    # id (
                    |*   id


函数调用(CALL)  ->*  id ( 参数 )

参数    ->*  值
        ->*  参数 , 值

变量定义语句    ->  id id ;

变量赋值定义语句    ->  id id = 值 ;

RETURN-语句 ->*  return 值 ;

值语句  ->*  值 ; # 值的根包含可执行tag

```