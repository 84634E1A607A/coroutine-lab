# Coroutine Lab ICS THU CS

实现了一个支持多实例的 CoroutinePool. 仅支持 x64 Microsoft ABI 与 SysV ABI.

Features:

- 多实例
- 等待队列, 就绪队列, 删除队列
- 复用栈空间

Restrictions:

- 暂无法支持动态栈空间 (因为存储当前携程实例的位置需要在编译时已知)

Performance:

```text
--# ./test -b 4
Size: 4294967296
Loops: 1000000
Batch size: 4
Initialization done
naive: 1100.53 ns per search, 34.39 ns per access
coroutine batched: 1332.71 ns per search, 41.65 ns per access
```

```text
PS coroutine> .\a.exe -b 4
Size: 0
Loops: 1000000
Batch size: 4
Initialization done
naive: 987.68 ns per search, 30.87 ns per access
coroutine batched: 1158.61 ns per search, 36.21 ns per access
```
