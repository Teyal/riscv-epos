# boot.s
# bootloader

.option norvc
.section .data
.global TOTAL_NUM_HARTS
TOTAL_NUM_HARTS: .byte 1

.section .text.init
.global _start
_start:
	csrr t0, mhartid #reading CSR mhartid
	bnez t0, 3f
	csrw satp, zero # seta supervisor address translation and protection register to 0. MMU related
.option push
.option norelax
	la gp, _global_pointer
.option pop
	la a0, _bss_start
	la a1, _bss_end
	bgeu a0, a1, 2f
1:
	sd zero, (a0) # store doubleword - guarda "duas palavras 32bits = 64"
	addi a0, a0, 8 # incrementa 64 bits até chegar no fim da bss
	bltu a0, a1, 1b
2:
	# li		t5, 0xffff;
	# csrw	medeleg t5
	# csrw	mideleg, t5
	la		sp, _stack_end
	li		t0, (0b11 << 11) | (1 << 7) | (1 << 3)
	# [11,12] = mpp = 11 => MACHINE PRIVILEDGE
	# 7 = MPIE = 1 => bit do interrupt ativo
	# 3 = MIE = 1 => ativo interrupcao nivel maquina
	csrw	mstatus, t0 # load in mstatus (machine status register) (o atual estado da thread) os op
	la		t1, kmain
	csrw	mepc, t1 # carrega no registrador de pc_exception endereco da kmain
	la		t2, m_trap_vector 
	csrw	mtvec, t2 #carrega em no reg trap_handler o endereco do trap_handler
	li		t3, (1 << 3) | (1 << 7) | (1 << 11)
	csrw	mie, t3 #carrega no registrador machine interrupt enable o padrao 
	# msie = 1 =>	habilita interrupcao maquina 
	# mtie = 1 => timer modo maquina habilitado
	# meie = 1 => habilita interrupcoes externas
	la		ra, 4f
	mret	
3:
	# Parked harts go here. We need to set these
	# to only awaken if it receives a software interrupt,
	# which we're going to call the SIPI (Software Intra-Processor Interrupt).
	# We only use these to run user-space programs, although this may
	# change.
	la		t0, TOTAL_NUM_HARTS
	lb		t1, 0(t0)
	addi	t1, t1, 1
	sb		t1, 0(t0)

	li		t0, 1 << 3
	csrw	mstatus, t0
	li		t1, 1 << 3
	csrw	mie, t1
	la		t2, kmain
	csrw	mtvec, t2
	# Waiting loop. An IPI will cause the other harts to
	# wake up and start executing.
4:
	wfi
	j 4b
