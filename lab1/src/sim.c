#include "shell.h"
#include <stdio.h>

// 宏定义，简化编码
#define NEXT_REG(i) NEXT_STATE.REGS[(i)]
#define CUR_REG(i) CURRENT_STATE.REGS[(i)]
#define NEXT_PC NEXT_STATE.PC
#define CUR_PC CURRENT_STATE.PC
#define CUR_HI CURRENT_STATE.HI
#define CUR_LO CURRENT_STATE.LO
#define NEXT_HI NEXT_STATE.HI
#define NEXT_LO NEXT_STATE.LO

// 将提取出的指令的各个部分保留在全局变量中，方便后续操作
uint8_t op, rs, rt, rd, shamt, funct;
uint16_t immediate;
uint32_t instruction, target;

// 解析指令
void extract_code_format()
{
    // 通过位移和位与解析指令
    // 如(instruction >> 26) & 0x3F，先将指令右移26位，再位与上0b111111，即从26位开始取出前面的6位指令，即26--31位
    op = (instruction >> 26) & 0x3F;
    rs = (instruction >> 21) & 0x1F;
    rt = (instruction >> 16) & 0x1F;
    rd = (instruction >> 11) & 0x1F;
    immediate = instruction & 0xFFFF;
    target = instruction & 0x3FFFFFF;
    shamt = (instruction >> 6) & 0x1F;
    funct = instruction & 0x3F;
}

// 处理R型指令
void r_fmt_ins_exec()
{
    switch (funct) {
    case 0x00: // SLL指令
        // 左移指令，直接使用<<即可
        NEXT_REG(rd) = CUR_REG(rt) << shamt;
        break;
    case 0x02: // SRL指令
        // 右移指令，直接使用>>即可
        NEXT_REG(rd) = CUR_REG(rt) >> shamt;
        break;
    case 0x03: // SRA指令
        // 算数右移指令，先将数据转为有符号整数，再右移即可
        NEXT_REG(rd) = (int32_t)CUR_REG(rt) >> shamt;
        break;
    case 0x04: // SLLV指令
        // 将tr寄存器的值左移，左移个数为rs寄存器的后五位，保存在rd寄存器中
        NEXT_REG(rd) = CUR_REG(rt) << (CUR_REG(rs) & 0x1F);
        break;
    case 0x06: // SRLV指令
        // 将tr寄存器的值右移，右移个数为rs寄存器的后五位，保存在rd寄存器中
        NEXT_REG(rd) = CUR_REG(rt) >> (CUR_REG(rs) & 0x1F);
        break;
    case 0x07: // SRAV指令
        // 类似SRLV指令，不过是算数右移，所以将rt寄存器的值转为有符号整数
        NEXT_REG(rd) = (int32_t)CUR_REG(rt) >> (CUR_REG(rs) & 0x1F);
        break;
    case 0x08: // JR指令
        // 跳转指令，将下一个PC设置为rs寄存器的值
        NEXT_PC = CUR_REG(rs);
        break;
    case 0x09: // JALR指令
        // 使用rd保存下一PC的值，然后跳转到rs寄存器指定的地址
        // 如果rs寄存器的低2位不为0则会出现地址异常，lab1暂不实现
        NEXT_REG(rd) = NEXT_PC;
        NEXT_PC = CUR_REG(rs);
        break;
    case 0x0c: // SYSCALL指令
        if (CUR_REG(2) == 0x0a)
            RUN_BIT = FALSE;
        break;
    case 0x20: // ADD指令
        // 加法指令，需要检测是否溢出，lab1暂时不实现
        NEXT_REG(rd) = CUR_REG(rs) + CUR_REG(rt);
        break;
    case 0x21: // ADDU指令
        // 加法指令，不检测溢出异常
        NEXT_REG(rd) = CUR_REG(rs) + CUR_REG(rt);
        break;
    case 0x22: // SUB指令
        // 减法指令
        // 要捕获整数溢出异常，暂不实现
        NEXT_REG(rd) = CUR_REG(rs) - CUR_REG(rt);
        break;
    case 0x23: // SUBU指令
        // 不捕获异常的减法指令
        NEXT_REG(rd) = CUR_REG(rs) - CUR_REG(rt);
        break;
    case 0x24: // AND指令
        // 位与指令
        NEXT_REG(rd) = CUR_REG(rs) & CUR_REG(rt);
        break;
    case 0x25: // OR指令
        // 位或指令
        NEXT_REG(rd) = CUR_REG(rs) | CUR_REG(rt);
        break;
    case 0x26: // XOR指令
        // 异或指令
        NEXT_REG(rd) = CUR_REG(rs) ^ CUR_REG(rt);
        break;
    case 0x27: // NOR指令
        // 或非指令
        NEXT_REG(rd) = ~(CUR_REG(rs) | CUR_REG(rt));
        break;
    case 0x2A: // SLT指令
        // 小于则置位，处理有符号数
        NEXT_REG(rd) = (int32_t)CUR_REG(rs) < (int32_t)CUR_REG(rt);
        break;
    case 0x2B: // SLTU指令
        // 小于则置位，处理无符号数
        NEXT_REG(rd) = CUR_REG(rs) < CUR_REG(rt);
        break;
    case 0x18: // MULT指令
        // 有符号乘法指令，将结果的低32位保存在NEXT_LO，高32位保存在NEXT_HI
        NEXT_HI = ((int64_t)CUR_REG(rs) * (int64_t)CUR_REG(rt)) >> 32;
        NEXT_LO = ((int64_t)CUR_REG(rs) * (int64_t)CUR_REG(rt)) & 0xFFFFFFFF;
        break;
    case 0x19: // MULTU指令
        // 无符号乘法指令，将结果的低32位保存在NEXT_LO，高32位保存在NEXT_HI
        NEXT_HI = ((uint64_t)CUR_REG(rs) * (uint64_t)CUR_REG(rt)) >> 32;
        NEXT_LO = ((uint64_t)CUR_REG(rs) * (uint64_t)CUR_REG(rt)) & 0xFFFFFFFF;
        break;
    case 0x1A: // DIV指令
        // 有符号除法指令，NEXT_LO保存除数，NEXT_HI保存余数
        if (CUR_REG(rt) != 0) {
            NEXT_LO = (int32_t)CUR_REG(rs) / (int32_t)CUR_REG(rt);
            NEXT_HI = (int32_t)CUR_REG(rs) % (int32_t)CUR_REG(rt);
        } else {
            printf("除数不能为0! ");
        }
        break;
    case 0x1B: // DIVU指令
        // 无符号除法指令，NEXT_LO保存除数，NEXT_HI保存余数
        if (CUR_REG(rt) != 0) {
            NEXT_LO = CUR_REG(rs) / CUR_REG(rt);
            NEXT_HI = CUR_REG(rs) % CUR_REG(rt);
        } else {
            printf("除数不能为0! ");
        }
        break;
    case 0x10: // MFHI指令
        // 将HI的值赋值给rd寄存器
        NEXT_REG(rd) = CUR_HI;
        break;
    case 0x12: // MFLO指令
        // 将LO的值赋值给rd寄存器
        NEXT_REG(rd) = CURRENT_STATE.LO;
        break;
    case 0x11: // MTHI指令
        // 将rs寄存器的值赋值给HI
        NEXT_HI = CUR_REG(rs);
        break;
    case 0x13: // MTLO指令
        // 将rs寄存器的值赋值给LO
        NEXT_LO = CUR_REG(rs);
        break;
    default:
        printf("未定义指令！\n");
    }
}

// 处理分支跳转的I型指令
void i_fmt_br_ins_exec()
{
    switch (rt) {
    case 0x00: // BLTZ指令
        // 当rs寄存器的值<0时跳转
        if ((int32_t)CUR_REG(rs) < 0)
            // 字节寻址，将immediate左移2位,并在PC+4基础上偏移，其中NEXT_PC已经PC+4
            NEXT_PC += ((int32_t)(int16_t)immediate << 2);
        break;
    case 0x01: // BGEZ指令
        // 当rs寄存器的值>=0时跳转
        if ((int32_t)CUR_REG(rs) >= 0)
            // 字节寻址，将immediate左移2位,并在PC+4基础上偏移，其中NEXT_PC已经PC+4
            NEXT_PC += ((int32_t)(int16_t)immediate << 2);
        break;
    case 0x10: // BLTZAL指令
        // 当rs寄存器的值<0时跳转，同时将下一PC写入寄存器$31
        if ((int32_t)CUR_REG(rs) < 0) {
            // 将PC+4的值保存在$31寄存器
            NEXT_REG(31) = NEXT_PC;
            // 字节寻址，将immediate左移2位,并在PC+4基础上偏移，其中NEXT_PC已经PC+4
            NEXT_PC += ((int32_t)(int16_t)immediate << 2);
        }
        break;
    case 0x11: // BGEZAL指令
        // 当rs寄存器的值>=0时跳转，同时将PC+4写入寄存器$31
        if ((int32_t)CUR_REG(rs) >= 0) {
            // 将PC+4的值保存在$31寄存器
            NEXT_REG(31) = NEXT_PC;
            // 字节寻址，将immediate左移2位,并在PC+4基础上偏移，其中NEXT_PC已经PC+4
            NEXT_PC += ((int32_t)(int16_t)immediate << 2);
        }
        break;
    default:
        printf("未定义指令！\n");
    }
}

// 处理J型指令
void j_fmt_ins_exec()
{
    switch (op) {
    case 0x02: // J指令
        // 无条件跳转，PC的高四位不变，低28位由target给出，其中NEXT_PC已经PC+4
        NEXT_PC = (NEXT_PC & 0xF0000000) | (target << 2);
        break;
    case 0x03: // JAL指令
        // 将PC+4的值保存在$31寄存器
        NEXT_REG(31) = CUR_PC + 4;
        // 无条件跳转，PC的高四位不变，低28位由target给出，其中NEXT_PC已经PC+4
        NEXT_PC = (NEXT_PC & 0xF0000000) | (target << 2);
        break;
    default:
        printf("未定义指令！\n");
    }
}

// 处理I型指令
void i_fmt_ins_exec()
{
    uint32_t address = (uint32_t)(int32_t)(int16_t)immediate + CUR_REG(rs); // 计算address
    uint32_t address_aligned = address & 0xfffffffc;                        // address & 0xfffffffc使address4字节对齐
    uint32_t address_data = mem_read_32(address_aligned);                   // 读取对齐后address的位置

    switch (op) {
    case 0x06: // BLEZ指令
        // 小于等于则分支，立即数左移两位并符号拓展,其中NEXT_PC已经PC+4
        if ((int32_t)CUR_REG(rs) <= 0)
            NEXT_PC += ((int32_t)(int16_t)immediate << 2);
        break;
    case 0x07: // BGTZ指令
        // 大于则分支，立即数左移两位并符号拓展,其中NEXT_PC已经PC+4
        if ((int32_t)CUR_REG(rs) > 0)
            NEXT_PC += ((int32_t)(int16_t)immediate << 2);
        break;
    case 0x08: // ADDI指令
        // 加立即数指令，要进行符号拓展和溢出检查
        NEXT_REG(rt) = CUR_REG(rs) + (int32_t)(int16_t)immediate;
        break;
    case 0x09: // ADDIU指令
        // 加立即数指令，要进行符号拓展，不进行溢出检查
        NEXT_REG(rt) = CUR_REG(rs) + (int32_t)(int16_t)immediate;
        break;
    case 0x0C: // ANDI指令
        // 立即数位与指令，进行无符号拓展
        NEXT_REG(rt) = CUR_REG(rs) & (uint32_t)immediate;
        break;
    case 0x0D: // ORI指令
        // 立即数位或指令，进行无符号拓展
        NEXT_REG(rt) = CUR_REG(rs) | (uint32_t)immediate;
        break;
    case 0x0E: // XORI指令
        // 立即数异或指令，进行无符号拓展
        NEXT_REG(rt) = CUR_REG(rs) ^ (uint32_t)immediate;
        break;
    case 0x0A: // SLTI指令
        // 小于立即数则置位，有符号
        NEXT_REG(rt) = (int32_t)CUR_REG(rs) < (int32_t)(int16_t)immediate;
        break;
    case 0x0B: // SLTIU指令
        // 小于立即数则置位，符号拓展，以无符号数形式进行比较
        NEXT_REG(rt) = CUR_REG(rs) < (uint32_t)(int32_t)(int16_t)immediate;
        break;
    case 0x0F: // LUI指令
        // 加载立即数的高16位，低16位用0填充
        NEXT_REG(rt) = (uint32_t)immediate << 16;
        break;
    case 0x04: // BEQ指令
        // 相等则跳转，
        if (CUR_REG(rs) == CUR_REG(rt)) {
            NEXT_PC += ((int32_t)(int16_t)immediate << 2);
        }
        break;
    case 0x05: // BNE指令
        // 不等则跳转
        if (CUR_REG(rs) != CUR_REG(rt)) {
            NEXT_PC += ((int32_t)(int16_t)immediate << 2);
        }
        break;
    case 0x23: // LW指令
        // 取字指令
        if (address & 0x3) { // 当地址的低两位不是零时发生地址错误
            printf("地址错误!\n");
        } else {
            NEXT_REG(rt) = mem_read_32(address);
        }
        break;
    case 0x2B: // SW指令
        // 存字指令
        if (address & 0x3) { // 当地址的低两位不是零时发生地址错误
            printf("地址错误!\n");
        } else {
            mem_write_32(address, CUR_REG(rt));
        }
        break;
    case 0x28: // SB指令
        // 计算要加载位置的偏移量
        int16_t shift_sb = (address & 0x3) << 3;
        // 清除原始数据的相应8位位置
        address_data &= ~(0xFF << shift_sb);
        // 将rt寄存器的低8位值存储到原始数据的相应位置
        address_data |= ((CUR_REG(rt) & 0xff) << shift_sb);
        // 将修改后的数据写回到内存
        mem_write_32(address_aligned, address_data);
        break;
    case 0x29: // SH指令
        // 存半字指令
        if (address & 0x1) { // 当地址的最低位不是零时发生地址错误
            printf("地址错误!\n");
        } else {
            // 计算要加载位置的偏移量
            int16_t shift_sh = (address & 0x2) << 3;
            // 清除原始数据的相应16位位置
            address_data &= ~(0xFFFF << shift_sh);
            // 将rt寄存器的低16位值存储到原始数据的相应位置
            address_data |= ((CUR_REG(rt) & 0xffff) << shift_sh);
            // 将修改后的数据写回到内存
            mem_write_32(address_aligned, address_data);
        }
        break;
    case 0x21: // LH指令
        // 取半字指令，符号拓展
        if (address & 0x1) { // 当地址的最低位不是零时发生地址错误
            printf("地址错误!\n");
        } else {
            int16_t shift_lh = (address & 0x2) << 3;
            NEXT_REG(rt) = (int32_t)(int16_t)(((address_data & (0xffff << shift_lh)) >> shift_lh));
        }
        break;
    case 0x25: // LHU指令
        // 取半字指令，无符号拓展
        if (address & 0x1) { // 当地址的最低位不是零时发生地址错误
            printf("地址错误!\n");
        } else {
            int16_t shift_lhu = (address & 0x2) << 3;
            NEXT_REG(rt) = (uint32_t)(((address_data & (0xffff << shift_lhu)) >> shift_lhu));
        }
        break;
    case 0x20: // LB指令
        // 取字节指令，符号拓展
        int16_t shift_lb = (address & 0x3) << 3;
        NEXT_REG(rt) = (int32_t)(int8_t)(((address_data & (0xff << shift_lb)) >> shift_lb));
        break;
    case 0x24: // LBU指令
        // 取字节指令，无符号拓展
        int16_t shift_lbu = (address & 0x3) << 3;
        NEXT_REG(rt) = (uint32_t)(((address_data & (0xff << shift_lbu)) >> shift_lbu));
        break;
    default:
        printf("未定义指令！\n");
    }
}

// 分发并执行指令
void code_dispatch_and_execute()
{
    // 根据op对指令进行分发
    if (op == 0x00)
        r_fmt_ins_exec();
    else if (op == 0x01)
        i_fmt_br_ins_exec();
    else if (op == 0x02 || op == 0x03)
        j_fmt_ins_exec();
    else
        i_fmt_ins_exec();
}

void process_instruction()
{
    /* execute one instruction here. You should use CURRENT_STATE and modify
     * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     * access memory. */

    // 获取当前指令并打印
    instruction = mem_read_32(CUR_PC);
    printf("当前指令: %u\n", instruction);

    // 解析当前指令
    extract_code_format();

    // PC加4，执行下一条指令
    NEXT_PC = CUR_PC + 4;

    // 分发并执行指令
    code_dispatch_and_execute();
}