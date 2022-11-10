# tlibcxx_vector_test

# About
Intel SGXの開発ツール、SGX SDKに同梱されている独自ライブラリ<vector>に関する性能調査。

# How to use

```
$ wget https://download.01.org/intel-sgx/latest/linux-latest/distro/ubuntu20.04-server/sgx_linux_x64_sdk_2.17.100.3.bin
$ chmod 777 sgx_linux_x64_sdk_2.17.100.3.bin
$ ./sgx_linux_x64_sdk_2.17.100.3.bin
[yes]
$ source {ENVIRONMENT-PATH}   // indicated by green text. 
$ cd tlibcxx_vector_test
$ make
$ ./app
```
  
# Result

[計測条件]<br>
要素数10,000,000件のvector配列に対して、indexを用いた要素検索(normal_loop)とiteratorを用いた要素検索(iterator_loop)を全てのデータに対して昇順に行う。<br>
上記の処理をenclave内部、外部でそれぞれ10回行い、平均を算出している。<br>
また、実行時間はクロックカウンタを用いて計測している。<br>
[計測結果]<br>
SGX SDKの独自ライブラリの方が、iteratorを用いた配列検索に関して約3倍程度性能が向上していた。<br>
以下は実行結果の一例である。

```
                enclave         not enclave     (enclave)/(not enclave)
normal_loop:0   59441281        71244007        0.834334
normal_loop:1   59713795        63506639        0.940276
normal_loop:2   60985603        63019235        0.96773
normal_loop:3   59932675        64247463        0.932841
normal_loop:4   64555938        63153103        1.02221
normal_loop:5   65869326        63465598        1.03787
normal_loop:6   61573876        63564625        0.968681
normal_loop:7   59419225        64329121        0.923675
normal_loop:8   60065067        64641766        0.929199
normal_loop:9   64901190        64579853        1.00498
[average]       61645797        64575141        0.954637

iterator_loop:0 145543175       429665159       0.338736
iterator_loop:1 125856019       435758753       0.28882
iterator_loop:2 126630514       438800066       0.288584
iterator_loop:3 127506612       434118977       0.293714
iterator_loop:4 134973343       434536362       0.310615
iterator_loop:5 132366866       436329588       0.303364
iterator_loop:6 126555048       426967031       0.296405
iterator_loop:7 131267102       436834277       0.300496
iterator_loop:8 130505645       433826271       0.300825
iterator_loop:9 130368411       433752864       0.300559
[average]       131157273       434058934       0.302165
```
