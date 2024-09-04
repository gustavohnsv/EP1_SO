# Dia 03/09/2024

Já havia instalado o projeto pela página da tarefa disponível pelo E-disciplinas, na matéria de Sistemas Operacionais, porém fiz um *fork* do projeto original do Github para futuramente poder comparar entre os *commits*. O projeto original está disponível <a href="https://github.com/mit-pdos/xv6-public">neste link</a>. Vale ressaltar também que executei ambos os projetos na minha máquina pelo terminal por meio do comando `make qemu` e não tive nenhum problema. Algumas informações sobre o sistema abaixo:

- Versão utilizada do GCC: 11.4.0
- Versão utilizada do GNU Make: 4.3
- Versão utilizada do GNU ld: 2.42
- Sistema operacional: Ubuntu (Linux)
- Versão do sistema operacional: 24.04 LTS
- Arquitetura do sistema operacional: x86_64 
- É uma máquina virtual?: Não

Após ter pesquisado um pouco em fórums da internet, bem como visto em alguns vídeos no Youtube, o primeiro passo para poder adicionar a chamada é editando alguns arquivos, como no `syscall.h`, `syscall.c`, `sysproc.c`, `usys.S`, `proc.h`, `user.h` adicionando:

```
    ..arquivo syscall.h
    #define SYS_getreadcount 22
```
```
    ..arquivo syscall.c
    extern int sys_getreadcount(void);
    ...
    [SYS_getreadcount] sys_getreadcount,
```
```
    ..arquivo sysproc.c (apenas o protótipo da função)
    int
    sys_getreadcount(void)
    {
    return 0;
    }
```
```
    ..arquivo usys.S
    SYSCALL(getreadcount)
```
```
    ..arquivo proc.h
    int reads;
```
```
    ..arquivo user.h
    int getreadcount(void);
```

Adicionei também um arquivo chamado `getreadcount.c` para me auxiliar nos testes da chamada. De modo que ficou:
```
    // Test new system call getreadcount()

    #include "types.h"
    #include "stat.h"
    #include "user.h"

    int
    main(void)
    {
        int n = getreadcount();
        printf(1, "read count: %d\n", n);
        exit();
    }
```

Logo após, adicionei o programa de teste no Makefile para que fosse compilado junto com os outros programas
```
    ..arquivo Makefile
    	_getreadcount\
```

Modifiquei a função `void syscall(void)` em `syscall.c`, para que toda vez que uma chamada `sys_read()` fosse feita, ele chamasse também a chamada `sys_getreadcount()`. Dessa forma, ficou:
```
    ..arquivo syscall.c
    if (num == SYS_read) {
        curproc->tf->eax = sys_read();
        sys_getreadcount();
        return;
    }
```

Adicionei o arquivo de teste chamado `test_1.c` para testar se a função está funcionando. O resultado foi:
```
    ..saída do xv6
    $ test_1
    XV6_TEST_OUTPUT 1 2 1001
```  

Modifiquei para o incremento ser realizado na chamada `sys_read()` e a chamada `sys_getreadcount()` apenas exibe o contador.