# Dia 03/09/2024

Já havia instalado o projeto pela página da tarefa disponível pelo E-disciplinas, na matéria de Sistemas Operacionais, porém fiz um *fork* do projeto original do Github para futuramente poder comparar entre os *commits*. O projeto original está disponível [neste link](https://github.com/mit-pdos/xv6-public). Vale ressaltar também que executei ambos os projetos na minha máquina pelo terminal por meio do comando `make qemu` e não tive nenhum problema. Algumas informações sobre o sistema abaixo:

- Versão utilizada do GCC: 11.4.0
- Versão utilizada do GNU Make: 4.3
- Versão utilizada do GNU ld: 2.42
- Sistema operacional: Ubuntu (Linux)
- Versão do sistema operacional: 24.04 LTS
- Arquitetura do sistema operacional: x86_64 
- É uma máquina virtual?: Não

Após ter pesquisado um pouco em fórums da internet, bem como visto em alguns vídeos no Youtube, o primeiro passo para poder adicionar a chamada é editando alguns arquivos, como no `syscall.h`, `syscall.c`, `sysproc.c`, `usys.S`, `proc.h`, `user.h` adicionando:

```c
// arquivo syscall.h
#define SYS_getreadcount 22
```

```c
// arquivo syscall.c
extern int sys_getreadcount(void);
...
[SYS_getreadcount] sys_getreadcount,
```

```c
// arquivo sysproc.c (apenas o protótipo da função)
int
sys_getreadcount(void)
{
    return 0;
}
```

```c
// arquivo usys.S
SYSCALL(getreadcount)
```

```c
// arquivo proc.h
int reads;
```

```c
// arquivo user.h
int getreadcount(void);
```

Adicionei também um arquivo chamado `getreadcount.c` para me auxiliar nos testes da chamada. De modo que ficou:

```c
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

Logo após, adicionei o programa de teste no Makefile para que fosse compilado junto com os outros programas:

```makefile
// arquivo Makefile
    _getreadcount\
```

Modifiquei a função `void syscall(void)` em `syscall.c`, para que toda vez que uma chamada `sys_read()` fosse feita, ele chamasse também a chamada `sys_getreadcount()`. Dessa forma, ficou:

```c
// arquivo syscall.c
if (num == SYS_read) {
    curproc->tf->eax = sys_read();
    sys_getreadcount();
    return;
}
```

Adicionei o arquivo de teste chamado `test_1.c` para testar se a função está funcionando. O resultado foi:

```makefile
// arquivo Makefile
    _test_1\
```

```
// saída do xv6
$ test_1
XV6_TEST_OUTPUT 0 1 1000
```

Modifiquei para o incremento ser realizado na chamada `sys_read()` e a chamada `sys_getreadcount()` apenas exibe o contador.

# Dia 04/09/2024

Esqueci de documentar no dia anterior, mas a modificação feita na chamada foi:

```c
// arquivo sysfile.c
myproc()->reads++;
```

Dessa forma, conferi com o ChatGPT (uma fonte não muito confiável), para ver se o novo resultado fazia um pouco de sentido e obtive a seguinte resposta ao executar o teste (A saída esperada por ele e a saída do programa coincidiram):

> Nota pessoal: Percebi só depois de baixar o segundo teste que as saídas esperadas estavam no próprio arquivo (Bem atento!)

```bash
// saída xv6
$ test_1
XV6_TEST_OUTPUT 0 1 1000
```

Agora adicionei o arquivo de teste chamado `test_2.c` para testar se a função continua funcionando ou precisa de modificações no que se trata da lógica, e o resultado foi:

```makefile
// arquivo Makefile
    _test_2\
```

```
// saída xv6
$ test_2
XV6_TEST_OUTPUT 100000
```

Como esperado, a saída não condizia, pois o programa fazia um *fork* e cada *reads* estava sendo armazenado em cada processo, ao invés de estar sendo compartilhado. Uma maneira simples de resolver esse problema é introduzir cuidadosamente uma variável global que será acessada por qualquer processo.

> Nota pessoal: Estou fazendo testes, criando uma variável global `static int global_reads` e começando a partir do `0`, mas as saídas tanto do primeiro quanto do segundo teste começaram a divergir das saídas esperadas. Estou tentando "transformar" as leituras de cada processo em algo global, usando locks para evitar que dois ou mais processos mexam na mesma variável ao mesmo tempo.

> Nota pessoal 2: Consegui implementar uma espécie de lock quando a chamada `sys_read` é feita, mas não sei ao certo se ainda está funcionando. Adicionei alguns `cprintf()` para verificar as leituras que eram feitas e pude perceber que quando eu executava um comando, ele começava uma outra contagem e depois voltava. Testei com a chamada `ls`, então creio que o xv6 está criando um outro processo para essa chamada, começando uma outra contagem do 0, e quando a chamada termina, ele volta para o processo em que eu estava.

> Nota pessoal 3: Rodei tanto o primeiro teste quanto outro chamado `zombie` e percebi que um criava um novo processo para ser executado enquanto o outro não, respectivamente. Percebi também que alguns *reads* são feitos antes de executar qualquer coisa de fato.

> Nota pessoal 4: Não consegui resolver o problema, aparentemente ainda está havendo concorrência entre os processos, o `global_reads` não é atualizado da forma que deveria. Tentarei usar alguma ferramenta de depuração para acompanhar o programa ou tentarei guardar as leituras em outro local que não seja dentro do processo (de uma maneira mais global do que atualmente), ou então tentar resolver essa questão do compartilhamento de chamadas globais em cada processo separadamente.

# Dia 05/09/2024

Depois de algumas tentativas do dia anterior, tentarei mudar a abordagem. Agora `sys_getreadcount()` irá retornar o número de leituras globais ao invés de exibir o número de leituras de cada processo.

Acho que consegui fazer. Durante alguns testes, fiz algumas mudanças nos arquivos `sysfile.c`, `sysproc.c`, `syscall.c`, `defs.h`:

```c
// arquivo sysfile.c
acquire(&readlock);
myproc()->reads = ++global_reads;
release(&readlock);
```

```c
// arquivo sysproc.c
int
sys_getreadcount(void)
{
    return global_reads;
}
```

```c
// arquivo syscall.c
volatile int global_reads = 0;
```

```c
// arquivo defs.h
extern volatile int global_reads;
```

> Nota pessoal: Muito provavelmente, tanto o compilador quanto o hardware estão garantindo que a operação de incremento, juntamente com a de atribuição, seja atômica, isto é, uma operação que não pode ser interrompida durante o processo e deve ser feita até o final. Na parte que tange o hardware, devido ao poder computacional oferecido, se torna um pouco difícil gerar situações de "gargalo".

> Nota pessoal 2: Esqueci de mencionar também que o `readlock` está sendo implementado em `trap.c` e listado em `defs.h`, da seguinte maneira:

> Nota pessoal 3: Acabei não necessitando utilizar o `gdb` para depuração do programa.

```c
// arquivo trap.c
struct spinlock readlock;
...
initlock(&readlock, "readlock");
```

```c
// arquivo defs.h
extern struct spinlock readlock;
```

Feito isso, os resultados de ambos os testes foram:

```bash
// saída xv6
$ test_1
XV6_TEST_OUTPUT 0 1 1000
$ test_2
XV6_TEST_OUTPUT 200000
```

# Conclusão

Creio que feito isso, a chamada foi implementada de maneira satisfatória. Em suma, as mudanças/implementações significativas foram: 

1. Declaração de uma variável global volátil no arquivo `syscall.c`, chamada `volatile int global_reads`, de maneira que qualquer chamada `read()` possa acessá-la; 
2. Cada processo guarda o número de chamadas de leituras globais, ou seja, chamadas que foram feitas por qualquer processo; 
3. Incremento da variável `global_reads` e atribuição logo em sequência da variável `myproc()->reads`, que supostamente, o compilador torna esse procedimento como atômico;
4. A chamada `getreadcount()` retorna a variável global citada anteriormente.
5. Implementação dos testes fornecidos.

# Referências

1. Acervo Lima. (n.d.). Sistema Operacional XV6: Adicionando uma nova chamada de sistema. Disponível em: https://acervolima.com/sistema-operacional-xv6-adicionando-uma-nova-chamada-de-sistema/

2. GeeksforGeeks. (n.d.). XV6 Operating System: Adding a New System Call. Disponível em: https://www.geeksforgeeks.org/xv6-operating-system-adding-a-new-system-call/

3. BridgeSign. (n.d.). Adding System Call to XV6 Kernel - Github Gist. Disponível em: https://gist.github.com/bridgesign/e932115f1d58c7e763e6e443500c6561

4. Yazan Hussnain. (n.d.). Adding System Call to XV6 Kernel - Github Post. Disponível em: https://github.com/YazanHussnain/Adding-system-call-to-xv6-kernel

5. Mahi12. (2019). Adding System Call in XV6. Medium. Disponível em: https://medium.com/@mahi12/adding-system-call-in-xv6-a5468ce1b463

6. Stack Overflow. (2021). Getting the process name and printing it out in xv6 [Post]. Disponível em: https://stackoverflow.com/questions/68182320/getting-the-process-name-and-printing-it-out-in-xv6-c

7. Youtube. (n.d.). Adding a System Call in XV6 - Video 1. Disponível em: https://www.youtube.com/watch?v=21SVYiKhcwM

8. Youtube. (n.d.). Adding a System Call in XV6 - Video 2. Disponível em: https://www.youtube.com/watch?v=SYRUMY9jqV4