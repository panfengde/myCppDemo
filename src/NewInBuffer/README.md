如何高效的批量克隆一批对象
在C++中，生成新的对象，我们往往使用new的方式。但是，在有的情节下，有比new效率高得多的方式。
使用buffer字节流

结果
```txt
int objNumbers = 20000;//一个batch生成20000个对象
int batchNumbers = 10000;//循环10000个批次
63623.1ms //传统new 花费时间
4146ms    // buffer 批量克隆花费的时间

差距非常大。而且随着每个批次数量的增增加，差距还会进一步拉大
```

