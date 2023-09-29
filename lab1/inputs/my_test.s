.data
    array: .space 16  # 用于存储数据的数组
    result: .word 0   # 用于存储计算结果的变量

.text
    main:
        # 初始化数据
        ori $t0, $zero, 10  # 将寄存器$t0初始化为10
        ori $t1, $zero, 3   # 将寄存器$t1初始化为3
        sw $t0, array       # 存储$t0的值到数组
        sw $t1, array + 4   # 存储$t1的值到数组的下一个位置

        # 加法
        lw $t2, array       # 从数组加载数据到$t2
        add $t3, $t0, $t1   # 计算$t0 + $t1
        add $t4, $t2, $t3   # 计算$t2 + $t3

        # 减法
        sub $t5, $t4, $t2   # 计算$t4 - $t2

        # 乘法
        mult $t5, $t1   # 计算$t5 * $t1

        # 除法
        div $t7, $t5, $t0   # 计算$t6 / $t0

        # 分支跳转
        beq $t7, $t1, equal
        bne $t7, $t1, not_equal

    equal:
        ori $t8, $zero, 1
        j end

    not_equal:
        ori $t8, $zero, 0

    end:
        # 存储计算结果到result变量
        sw $t8, result

        # 终止程序
        addiu $v0, $zero, 10
        syscall

