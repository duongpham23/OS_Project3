#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

int
sys_pgaccess(void)
{
  uint64 buf, abits, result_mask = 0;
  int page_num;
  struct proc *p = myproc();

  //Lay dia chi cua cac tham so cua ham pgaccess()
  argaddr(0, &buf);
  argint(1, &page_num);
  argaddr(2, &abits);

  //Duyet qua cac trang va tim kiem cac trang da duoc truy cap
  for (int i = 0; i < page_num; i++){
    if (i >= 64) break; //Gioi han so trang duoc duyet la 64 (64-bit)

    pte_t *pte = walk(p->pagetable, buf + i * PGSIZE, 0);

    if (pte == 0|| (*pte & PTE_V) == 0) continue; //Trang khong hop le thi bo qua

    if ((*pte & PTE_A) != 0){ //trang da duoc truy cap
      result_mask |= (1UL << i); //dung UL (64-bit) de khong lo bi tran` khi dich bit
      *pte &= ~PTE_A; //xoa bit PTE_A trong trang dang duoc duyet
    }
  }

  //Copy ket qua ra user space
  copyout(p->pagetable, abits, (char*)&result_mask, sizeof(result_mask));

  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
