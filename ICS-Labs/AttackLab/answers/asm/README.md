# answers/asm — 三个阶段的答案汇编与布局

两个版本，对应同一份程序的两份构建：

| 目录 | 对应程序 | 差异 |
| --- | --- | --- |
| `local/` | 本机的 `attack1`、`attack2`、`attack3` | `attack2` 的金丝雀固定为 `0x1122334455667700`（编译进 `.data`，每个学号相同） |
| `remote/` | 服务器复核用的构建 | `attack2` 的金丝雀由服务器按学号生成，本学号为 `0x8c84bbbba2d10100` |

阶段 1 和阶段 3 没有金丝雀，两份程序的指令、地址、缓冲区距离完全一致，
所以 `local/attack1.s` 与 `remote/attack1.s`、`local/attack3.s` 与
`remote/attack3.s` 汇编出的字节相同（源文件只有开头注释不同）；只有
`attack2.s` 不同（第 360 字节处的金丝雀值）。

每个文件都是可以直接汇编的 AT&T 汇编，按输入偏移从低到高排列：

- `.fill N, 1, 0x67` — N 个填充字节（0x67 = `'g'`）
- `.quad V` — 8 字节小端值（返回地址、金丝雀、ROP 数据）
- `push` / `mov` / `ret` — shellcode 指令

## 核对：汇编结果与已提交的答案一致

```bash
# attack1
tools/asmhex < answers/asm/local/attack1.s  > /tmp/a1.hex
diff <(tools/unhex < /tmp/a1.hex) <(tools/unhex < answers/attack1.txt)

# attack2（本地版与远程版分别核对）
tools/asmhex < answers/asm/local/attack2.s  > /tmp/a2l.hex
diff <(tools/unhex < /tmp/a2l.hex) <(tools/unhex < tmp/attack2.local.txt)
tools/asmhex < answers/asm/remote/attack2.s > /tmp/a2r.hex
diff <(tools/unhex < /tmp/a2r.hex) <(tools/unhex < answers/attack2.txt)

# attack3
tools/asmhex < answers/asm/local/attack3.s  > /tmp/a3.hex
diff <(tools/unhex < /tmp/a3.hex) <(tools/unhex < answers/attack3.txt)
```

## 复盘要点

- 三个程序都是静态链接、非位置无关：`objdump` 里的地址就是运行时地址。
- 缓冲区都在 `chat_turn` 的栈帧里，返回地址在 `%rbp+8`；返回地址距离缓冲区
  起点：阶段 1 = 456、阶段 2 = 368（再加 8 字节金丝雀）、阶段 3 = 296。
- `push %rsp; ret` 技巧：`ret` 进入 gadget 时 `%rsp` 正好指向 payload 里紧跟
  返回地址的下一段数据，不依赖 ASLR。
- 阶段 2 的金丝雀可以一字节一字节问出来：候选之后的字节仍保留真值，
  所以从左到右解；求解脚本是 `tools/find_canary.py`，8 字节共约 8×128 次请求。
