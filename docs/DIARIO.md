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

# Dia 04/09/2024

Esqueci de documentar no dia anterior, mas a modificação feita na chamada foi:

```
    ..arquivo sysfile.c
    myproc()->reads++;
```

Dessa forma, conferi com o ChatGPT (uma fonte não muito confiável), para ver se o novo resultado fazia um pouco de sentido e obtive a seguinte resposta ao executar o teste (A saída esperada por ele e a saída do programa coincidiram):

> Nota pessoal: Percebi só depois de baixar o segundo teste que as saídas esperadas estavam no próprio arquivo (Bem atento!)

```
    ..saída do xv6
    $ test_1
    XV6_TEST_OUTPUT 0 1 1000
```

Agora adicionei o arquivo de teste chamado `test_2.c` para testar se a função continua funcionando ou precisa de modificações no que se trata da lógica, e o resultado foi:

```
    ..saída do xv6
    $ test_2
    XV6_TEST_OUTPUT 100000
```

Como esperado, a saída não condizia, pois o programa fazia um *fork* e cada *reads* estava sendo armazenado em cada processo, ao invés de estar sendo compartilhado. Uma maneira simples de resolver esse problema é introduzido cuidadosamente uma variável global que será acessada por qualquer processo.

> Nota pessoal: Estou fazendo testes, criando uma varíavel global `static int global_reads` e começando a partir do `0`, mas as saídas tanto do primeiro quanto do segundo teste começaram a divergir das saídas esperadas. Estou tentando "transformar" as leituras de cada processo em algo global, usando locks para evitar de dois ou mais processos mexerem na mesma varíavel ao mesmo tempo.

> Nota pessoal 2: Consegui implementar uma espécie de lock para quando a chamada `sys_read` fosse feita, mas não sei ao certo se ainda está funcionando. Adicionando alguns `cprintf()` para verificar as leituras que eram feitas, pude perceber que quando eu executava um comando, ele começava uma outra contagem e depois voltava. Eu testei com a chamada `ls`, então creio que o xv6 está criando um outro processo para essa chamada, começando uma outra contagem do 0, e quando a chamada termina, ele volta para o processo em que eu estava

> Nota pessoal 3: Eu rodei tanto o primeiro teste quanto outro chamado `zombie`, e percebi que um criava um novo processo para ser executado enquanto outro não, respectivamente. Percebi também que alguns *reads* são feitos antes de executar qualquer coisa de fato

> Nota pessoal 4: Não consegui resolver o problema, aparentemente ainda está havendo concorrência entre os processos, o `global_reads` não é atualizado da forma que deveria. Tentarei usar alguma ferramente de depuração para acompanhar o programa ou tentarei de algum jeito ou guardar as leituras em outro local que não seja dentro do processo (de uma maneira mais global do que atualmente), ou então tentar resolver essa questão do compartilhamento de chamadas globais em cada processo separadamente