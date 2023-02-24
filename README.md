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

[注意]<br>
時間計測のためにCPUクロックに基づくTSCを利用しているため、使用しているCPUのクロック周波数にあった数値を`CLOCKS_PER_US`に入れる必要あり。

# Result

[計測条件]<br>
要素数10,000,000件のvector配列に対して、indexを用いた要素検索(index_loop)とiteratorを用いた要素検索(iterator_loop)を全てのデータに対して昇順に行う。<br>
上記の処理をenclave内部、外部でそれぞれ10回行い、平均を算出している。<br>
また、実行時間はクロックカウンタを用いて計測している。<br>
[計測結果]<br>
SGX SDKの独自ライブラリの方が、iteratorを用いた配列検索に関して約3倍程度性能が向上していた。<br>
以下は実行結果の一例である。

```
                tlibcxx STL     (STL)/(tlibcxx)
index_loop:0    20ms    24ms    1.16414
index_loop:1    23ms    26ms    1.10495
index_loop:2    21ms    24ms    1.13897
index_loop:3    21ms    24ms    1.16067
index_loop:4    22ms    24ms    1.10007
index_loop:5    20ms    26ms    1.30518
index_loop:6    20ms    24ms    1.19707
index_loop:7    20ms    25ms    1.25696
index_loop:8    20ms    25ms    1.23191
index_loop:9    20ms    25ms    1.24738
[average]       21ms    25ms    1.18802

iterator_loop:0 43ms    147ms   3.36403
iterator_loop:1 43ms    148ms   3.40649
iterator_loop:2 43ms    151ms   3.47205
iterator_loop:3 44ms    151ms   3.38393
iterator_loop:4 44ms    148ms   3.34448
iterator_loop:5 45ms    148ms   3.23264
iterator_loop:6 44ms    151ms   3.43313
iterator_loop:7 45ms    151ms   3.36047
iterator_loop:8 43ms    153ms   3.49598
iterator_loop:9 44ms    155ms   3.46518
[average]       44ms    150ms   3.39504
```
