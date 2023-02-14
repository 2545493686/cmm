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
        ->  函数定义    # id id ( id id
        |  IF-分支       #* if 
        |  变量定义语句     # id id ;
        |  变量赋值定义语句 # id id =
        |  RETURN-语句      # return
        |  赋值语句         # id =
        |*  函数调用语句     # id ( id ,|)

函数定义    ->  id id ( 参数 ) 块

参数声明    -> id id
            | 参数声明 , id id

块  ->  { 语句集合 }

语句集合    ->  语句
            |  语句集合 语句

IF-分支(IF) ->*  if ( 值 ) 块

值(VALUE)   ->*  间接式

# 左递归的表达式
间接式(IndirectExpr)   ->*  加减运算

比较式(Compare) ->*  加减运算 <|> 比较式
                ->*  加减运算

加减运算(AddSub)   ->*  乘除运算 +|- 加减运算
                    ->*  乘除运算

乘除运算(MulDiv)   ->*  直接式 *|/ 乘除运算 
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

RETURN-语句 ->  return 值 ;

赋值语句    ->  id = 值 ;

函数调用语句    ->*  函数调用 ;

```