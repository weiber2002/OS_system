# my_os — NachOS 自主實作完整指南

> 目標：**自己**從乾淨骨架把 NTHU OS 的 MP1~MP4 實作出來，不看別人的解法。
> 這份文件盡量把「背景 / 怎麼跑 / 每個資料夾在幹嘛 / 每份作業要實作什麼」講清楚。

---

## 目錄
0. [NachOS 是什麼（背景）](#0-nachos-是什麼背景)
1. [整體架構：三層抽象](#1-整體架構三層抽象)
2. [目錄結構逐一解說](#2-目錄結構逐一解說)
3. [環境與編譯執行流程](#3-環境與編譯執行流程)
4. [一個系統呼叫是怎麼跑完全程的](#4-一個系統呼叫是怎麼跑完全程的)
5. [四份作業要實作什麼（詳細）](#5-四份作業要實作什麼詳細)
6. [除錯技巧](#6-除錯技巧)
7. [建議的工作流程](#7-建議的工作流程)

---

## 0. NachOS 是什麼（背景）

**NachOS**（Not Another Completely Heuristic Operating System）是教學用的作業系統。
它本身是一支跑在你 Linux 上的**一般程式**（`nachos` 執行檔），但它內部：

- 實作了一個**作業系統核心**（thread、排程、記憶體管理、檔案系統、系統呼叫）；
- 內含一顆**MIPS CPU 模擬器**，可以真的「執行」編譯成 MIPS 的使用者程式。

所以你會遇到兩種「程式」：

| 類型 | 用什麼編 | 跑在哪 | 例子 |
|------|---------|--------|------|
| **Kernel（核心）** | 主機的 g++（x86） | 直接在 Linux 上 | `nachos` 本體，你改的 `.cc/.h` |
| **User program（使用者程式）** | 內建的 MIPS 交叉編譯器 | NachOS 的 MIPS 模擬器內 | `test/halt.c`、`test/add.c` |

**你在做的事** ＝ 修改 NachOS 的**核心程式碼**，讓使用者程式能正確地跑起來、能呼叫系統服務。

---

## 1. 整體架構：三層抽象

```
┌──────────────────────────────────────────────┐
│  你的 Linux 主機 (paslab60, x86-64)             │   ← 真實硬體 + 真實 OS
│                                                │
│   ┌────────────────────────────────────────┐  │
│   │  nachos 執行檔 (NachOS Kernel)           │  │   ← 你改的東西在這層
│   │   threads / userprog / filesys / ...    │  │
│   │                                         │  │
│   │   ┌─────────────────────────────────┐   │  │
│   │   │  machine/ = 模擬的硬體            │   │  │   ← 模擬的 CPU / 記憶體 / disk
│   │   │   MIPS CPU、主記憶體、TLB、timer  │   │  │
│   │   │                                 │   │  │
│   │   │   ┌─────────────────────────┐   │   │  │
│   │   │   │  User Program (MIPS)     │   │   │  │   ← test/*.c 編成的程式
│   │   │   │  halt / add / fileIO... │   │   │  │
│   │   │   └─────────────────────────┘   │   │  │
│   │   └─────────────────────────────────┘   │  │
│   └────────────────────────────────────────┘  │
└──────────────────────────────────────────────┘
```

- 使用者程式想要服務（開檔、印字…）→ 發出 **system call** → 陷入(trap)到 NachOS 核心的 `ExceptionHandler` → 核心處理完把結果放回暫存器 → 使用者程式繼續。
- 這條路徑就是 MP1 的主戰場。

---

## 2. 目錄結構逐一解說

你的乾淨工作區（複製自 `src/NachOS-4.0_MP1`）：

```
my_os/
├── README.md          ← 你正在看的這份
├── first_test.c       ← 第一支入門測試程式
└── MP1_work/          ← 乾淨、可編譯執行的 NachOS（你動手的地方）
    ├── code/          ← 所有原始碼與建置目錄（重點）
    ├── coff2noff/     ← 把 MIPS COFF 執行檔轉成 NachOS 的 NOFF 格式的工具
    └── usr/local/nachos/  ← ⭐ 內建的 MIPS 交叉編譯器（編使用者程式用）
```

### `code/` 內部（核心）

| 資料夾 | 角色 | 內含重點檔案 | 你會在哪份 MP 改它 |
|--------|------|------------|------------------|
| **`threads/`** | **核心 / 執行緒層**：OS 的心臟 | `thread.cc`(執行緒)、`scheduler.cc`(排程器)、`synch.cc`(鎖/號誌/條件變數)、`alarm.cc`(計時喚醒)、`kernel.cc`(系統啟動、命令列解析)、`main.cc`(進入點)、`switch.S`(組語做 context switch) | **MP3** 主戰場 |
| **`userprog/`** | **使用者程式支援層** | `exception.cc`(⭐系統呼叫進入點)、`addrspace.cc`(位址空間/分頁載入)、`syscall.h`(syscall 編號與介面)、`ksyscall.h`(核心端 syscall 實作)、`synchconsole.cc`(同步化 console I/O) | **MP1、MP2** 主戰場 |
| **`filesys/`** | **檔案系統層** | `filesys.cc`(檔案系統本體)、`filehdr.cc`(檔案標頭＝inode)、`directory.cc`(目錄)、`openfile.cc`(開啟中的檔案)、`pbitmap.cc`(磁碟空間點陣圖)、`synchdisk.cc`(同步化 disk) | **MP4** 主戰場 |
| **`machine/`** | **模擬硬體層**（一般不改，但要讀懂） | `mipssim.cc`(MIPS 指令模擬)、`machine.cc`(CPU/記憶體)、`translate.cc`(虛擬→實體位址轉換、TLB)、`interrupt.cc`(中斷)、`timer.cc`、`console.cc`、`disk.cc`、`network.cc` | MP2/MP3 少量會碰 |
| **`lib/`** | 工具函式庫 | `list.cc`(串列)、`bitmap.cc`(點陣圖)、`debug.cc`(除錯輸出)、`sysdep.cc`(和真實 OS 溝通)、`hash.cc` | 幾乎不改 |
| **`network/`** | 網路訊息傳遞 | `post.cc`(郵局模型) | 通常不改 |
| **`test/`** | **使用者測試程式**（會用 MIPS 交叉編譯） | `halt.c`、`add.c`、`fileIO_test1.c`、`consoleIO_test1.c`、`Makefile`、`start.S`(使用者程式啟動碼) | 會加自己的測試 |
| **`build.linux/`** | **Linux 的建置目錄** | `Makefile`(⭐在這裡 `make`)，產出 `nachos` | 在這裡 build |
| `build.macosx/` `build.cygwin/` | 其他平台的建置目錄 | | 用不到 |

### 關鍵檔案速記
- **`userprog/exception.cc`** → 所有系統呼叫從這裡進來（MP1 從這裡開始）。
- **`userprog/syscall.h`** → syscall 的「編號」與「使用者端函式原型」。
- **`userprog/ksyscall.h`** → syscall 的「核心端實作」（`SysOpen`、`SysRead`…）。
- **`threads/scheduler.cc`** → 排程演算法（MP3）。
- **`filesys/filehdr.cc`** → inode 結構（MP4 支援大檔案要改這裡）。

---

## 3. 環境與編譯執行流程

### 3.1 一次性設定（新複製一份 MP 時都要做）

paslab60 的 g++ 是 11.4，比 2022 年新、比較嚴格，直接編會在 `lib/list.cc` 之類的模板報錯。
**必做**：在建置 Makefile 的 `CFLAGS` 尾端加上 `-fpermissive`：

```bash
cd ~/nthu-operating-system/my_os/MP1_work
sed -i 's/-DCHANGED -m32/-DCHANGED -m32 -fpermissive/' code/build.linux/Makefile
```

### 3.2 編譯核心

```bash
cd ~/nthu-operating-system/my_os/MP1_work/code/build.linux
make depend      # 建立相依關係（第一次一定要做）
make             # 編譯，產出 ./nachos
```

### 3.3 執行使用者程式

> ⚠️ **這個 NTHU 版執行使用者程式的參數是 `-e`，不是通用 NachOS 的 `-x`！**
> 用錯參數它什麼都不會跑、機器會空轉停不下來（看起來像當機）。

```bash
./nachos -e ../test/halt
```

正確會印出：
```
Machine halting!
This is halt
Ticks: total 52, idle 0, system 40, user 12
Disk I/O: reads 0, writes 0
Console I/O: reads 0, writes 0
Paging: faults 0
Network I/O: packets received 0, sent 0
```

常用參數：

| 參數 | 用途 |
|------|------|
| `-e <file>` | 執行一支使用者程式（可多個 `-e` 同時跑，MP2 用） |
| `-d <flags>` | 開 debug 訊息，`-d +` 全開；`-d m` 只看機器指令 |
| `-u` | 列出所有可用參數 |
| `-rs <seed>` | 隨機化 context switch（測排程用） |

### 3.4 重新編譯使用者測試程式（交叉編譯）

交叉編譯器已內建在 `code/usr/local/nachos/`，所以可以自己改 `test/*.c` 再重建：

```bash
cd ~/nthu-operating-system/my_os/MP1_work/code/test
make            # 產出 halt / add / fileIO_test... 的 NachOS 執行檔
```

---

## 4. 一個系統呼叫是怎麼跑完全程的

以「使用者程式呼叫 `PrintInt(55)`」為例，這條路你一定要看懂（MP1 就是照這條路加新 syscall）：

```
1. 使用者程式 test/xxx.c 呼叫  PrintInt(55);
       │
2. test/start.S 裡的 stub：把 syscall 編號(SC_PrintInt=16)放進暫存器 r2，
   參數 55 放進 r4，然後執行 MIPS 的 `syscall` 指令
       │
3. MIPS 模擬器(machine/mipssim.cc)遇到 syscall 指令 → 觸發 SyscallException
       │
4. 控制權跳進  userprog/exception.cc 的 ExceptionHandler()
       │  int type = kernel->machine->ReadRegister(2);   // 讀出 16
       │  switch(type){ case SC_PrintInt: ... }
       │
5. 讀參數： val = kernel->machine->ReadRegister(4);      // 讀出 55
       │
6. 呼叫核心端實作(ksyscall.h)： SysPrintInt(val);
       │
7. ⭐ 把 PC 往前推 4 bytes（PrevPC/PC/NextPC），否則會無限重複同一個 syscall
       │
8. return → 回到使用者程式繼續執行
```

**MP1 你要做的**，就是仿照 `SC_PrintInt`、`SC_Create` 這兩個「已經寫好的範例」，
把 `SC_Open / SC_Read / SC_Write / SC_Close` 這四個 case 補進 `ExceptionHandler()`，
並在 `ksyscall.h`（核心端）與 `filesys/`（真正動檔案的地方）把對應函式接起來。

---

## 5. 四份作業要實作什麼（詳細）

> 以下只講**任務、入口、觀念**，不給任何人的解法。每份都先讀該 MP 資料夾裡的 Spec PDF。

### MP1 — System Call & File I/O
**主題**：讓使用者程式能透過系統呼叫操作檔案與 console。

- **現況**：`syscall.h` 裡 `SC_Open(6) / SC_Read(7) / SC_Write(8) / SC_Close(10)` 是**被註解掉**的；
  `SC_Halt / SC_Create / SC_PrintInt` 已經幫你做好當範例。
- **要做**：
  1. 打開 `syscall.h` 裡那四個 `#define` 的註解，並確認使用者端函式原型存在。
  2. 在 `userprog/exception.cc` 的 `ExceptionHandler()` 新增這四個 `case`：讀參數 → 呼叫核心函式 → 回寫結果到 r2 → **PC+4**。
  3. 在 `userprog/ksyscall.h` 實作 `SysOpen/SysRead/SysWrite/SysClose`，往下接到 `filesys/`。
  4. 在 `filesys/filesys.h`（FILESYS_STUB 版）維護一張「開啟檔案表」(OpenFileId → OpenFile*)。
- **要理解**：使用者傳進來的是**虛擬位址**，字串/緩衝區要用 `kernel->machine->mainMemory[addr]` 去存取。
- **驗證**：`./nachos -e ../test/consoleIO_test1`、`fileIO_test1`、`fileIO_test2`。

### MP2 — Multi-Programming（多程式）
**主題**：讓多支使用者程式**同時**載入記憶體、各自有獨立位址空間、互不干擾。

- **入口**：`userprog/addrspace.cc` 的 `Load()` / `Execute()`、`machine/translate.cc`。
- **現況**：預設一次只能好好跑一支，位址空間假設自己獨佔整塊記憶體。
- **要做**：
  1. 用一個**全域的 frame 使用表**（physical page bitmap）管理實體記憶體頁框。
  2. 每個 process 建立自己的 **page table**，`Load()` 時把它的程式碼/資料搬到「被配置到的實體頁」，而不是固定從 0 開始。
  3. 讓 `Translate()` 依各自的 page table 做虛擬→實體轉換。
  4. 結束時歸還頁框。
- **驗證**：同時 `-e prog1 -e prog2`，兩者結果都要正確、不互相覆蓋。

### MP3 — CPU Scheduling（多層回饋佇列）
**主題**：把預設排程換成三層佇列 + 老化。

- **入口**：`threads/scheduler.cc`（`ReadyToRun` / `FindNextToRun`）、`thread.cc`、`alarm.cc`、`kernel.cc`。
- **要做**（依 Spec，常見設定）：
  - **L1**：Preemptive **SRTN**（剩餘 burst time 最短優先），可搶佔。
  - **L2**：Preemptive **Priority**（優先權高先跑）。
  - **L3**：**Round-Robin**，time quantum 100 ticks。
  - **Aging（老化）**：thread 在 ready queue 等待每滿 1500 ticks，priority +10；priority 跨越門檻(50/100)時升級佇列。
  - 需估計/更新每個 thread 的 CPU burst 時間（用近似公式）。
- **要理解**：搶佔發生在 timer 中斷（`alarm.cc`）與 thread 狀態改變時。
- **驗證**：課程給的 scheduling 測資，比對 context switch 的時間點與順序。

### MP4 — File System（大檔案 + 多層目錄）
**主題**：把陽春檔案系統升級成能存大檔、能有子目錄。

- **入口**：`filesys/filehdr.cc`(inode)、`directory.cc`、`filesys.cc`、`pbitmap.cc`。
- **現況**：檔案標頭只有直接索引，單檔上限很小（約 4KB）；目錄只有一層、固定大小。
- **要做**：
  1. **大檔案**：改 `FileHeader` 為多層索引（間接索引/索引區塊），讓單檔可達 spec 要求（例如 32KB 以上）。
  2. **多層子目錄**：支援 `/a/b/c` 這種路徑解析；`Create/Open/Remove` 都要能吃絕對路徑。
  3. **動態目錄**：目錄大小可成長，不再寫死筆數。
  4. 可能要做的系統呼叫：`Create/Open/Read/Write/Close/Seek`，以及建目錄。
- **驗證**：建立巢狀目錄、寫入大檔再讀回逐 byte 比對。

---

## 6. 除錯技巧

- **分階段**：先讓它**編譯過** → 再**跑起來不當機** → 最後才追**正確性**。每加一小段就 `make` + 測。
- **看資料流**：在關鍵函式塞 `DEBUG(dbgSys, "xxx=" << val << "\n");` 或直接 `cout`，配 `-d +` 觀察。
- **常見雷**：
  - syscall 後忘了 **PC+4** → 無限迴圈同一個 syscall。
  - 忘記使用者傳的是**虛擬位址**，直接當指標用 → 亂讀記憶體。
  - `-x` 打成執行參數（要用 `-e`）→ 空轉當機。
- `./nachos -d +` 訊息量很大，可 `| tail -50` 或導到檔案再看。

---

## 7. 建議的工作流程

```bash
# (1) 為每份 MP 從乾淨底複製一份（以 MP2 為例，MP2/MP3 沿用 MP1 的底往下做）
cd ~/nthu-operating-system
cp -r my_os/MP1_work my_os/MP2_work
sed -i 's/-DCHANGED -m32/-DCHANGED -m32 -fpermissive/' my_os/MP2_work/code/build.linux/Makefile  # 若還沒加過

# (2) 讀 Spec PDF，搞懂測資與要實作的介面
# (3) build.linux 裡 make，先跑測資看「現在壞在哪」
# (4) 從入口檔案(exception.cc / scheduler.cc / filehdr.cc)順著資料流補 TODO
# (5) 小步實作、小步測試
```

**現在就開始**：先照第 3 節把 `MP1_work` 編起來、跑 `./nachos -e ../test/halt` 確認環境，
再跑 `./nachos -e ../test/fileIO_test1` 看**未實作前會壞在哪** —— 那就是你 MP1 的起點。
