## Compilando:computer:
**Conceda permissão de execução e compile com gcc **
```bash
chmod +x multicast.c
gcc multicast.c -lpthread

```
## Execução

Execute o arquivo a.out gerado pelo gcc com o respectivo Multicast IP e porta 
```bash
  ./a.out <MULTICAST IP> <MULTICAST PORT>

```
Exemplo

```bash
  ./a.out 239.255.10.10 9000
```
