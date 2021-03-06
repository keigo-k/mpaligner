# mpaligner とは

自動読み付与（発音付与）のための文字列アライメントツールです．
多対多のアライメントを提供し，与えられた2つの文字列の最小パターンを見つけます．
アライメント結果は未知語に対する読み付与などに使用されます．
部分的にアライメントの正解を与える部分的アノテーションやある特定の表記と読みの
対応付けを他の対応付けよりも有利にしたり禁止したりできます．
また，特殊な表記と読みの対応付け（[AAA,トリプルエー]など）を検出する機能があります．
ライセンスは GNU GPL です．

# mpaligner の詳細
mpaligner は部分文字列の共起関係を基に部分文字列のアライメントをとります．
例えば，

東 京	ト ウ キ ョ ウ
京 都	キ ョ ウ ト
大 阪	オ オ サ カ

という表記と読みの対データがあるとします．日本語が分からない人にとっては「東」の
読みは「ト」なのか「トウ」なのか「トウキョ」なのかわかりません．しかし，「東京」
と「京都」を見比べてみると，「京」がある時に読みに「キョウ」が出現していることが
わかります．このことから「京」の読みは「キョウ」と推定すると，「東」はおそらく
「トウ」であることが分かります．ここでは，この「京」と「キョウ」の関係を共起関係
と定義します．mpaligner は上記のような共起関係を用いて部分文字列の最小パターンを
見つけアライメントを行います．最終的に mpaligner によって以下のようなデータが得られます．

東|京|	ト:ウ|キ:ョ:ウ|
京|都|	キ:ョ:ウ|ト|
大阪|	オ:オ:サ:カ|

「|」はアライメントの区切りを表す文字です．
また，「:」は文字同士の結合を示す文字です．
大阪が分割されていないのは，今回の学習データ内には「大	オオ」や
「坂	サカ」のパターンが他の対データには存在せず，最小のパターンとして
「大阪」が選ばれたからです．このデータは読み付与モデルを構築する際の
学習データとして使用することができます．

mpaligner の他に，文字列同士のアライメントをとるプログラムとして m2m-aligner があります．
m2m-aligner の詳細な情報は以下にあります．

http://code.google.com/p/m2m-aligner/

m2m-aligner は1対1，2対1，1対2でアライメントを行うことを想定しており，
2対2以上のアライメントを行うためには，アライメントの制約を緩める必要があります．
しかし，アライメントの制約を緩めることで，小さい単位でアライメントがとれなくなり，
日本語などの2対2以上のアライメントを必要とする言語では読み付与の性能に悪影響を
与えます．そこで，mpaligner はアライメントの制約がない場合でも小さい単位での
アライメントを実現するためにアルゴリズムを工夫しています．
さらに，長い文字列でも計算できるようにlogによる計算を採用し，
高速化・小メモリ化も行っています．

# mpaligner のインストール方法
　以下の通りです．

```
  $ git clone git@github.com:keigo-k/mpaligner.git
  $ cd mpaligner
  $ make
　$ cp mpaligner <パスの通ったディレクトリ>
```

# mpaligner の使い方

使用例：
```
$ cat source/test.utf8.txt | ./script/separate_for_char.pl utf8 \
  source/joint_chars.utf8.txt > source/test.utf8.char_unit
$ mpaligner -i source/test.utf8.char_unit
```

このコマンドにより，test.utf8.char_unit をインプットデータとして，
アライメントを行います．アライメントの結果は test.utf8.char_unit.align に出力されます．
以下にオプションの説明を示します．

```
./mpaligner -i <string> [-o <string>] [-p <string>] [-ai <string>]
          [-ao <string>] [-output_used_mapping <string>] [-s]
          [-hs <int>] [-del] [-ins] [-rx <int>] [-ry <int>]
          [-uc <char>] [-sc <char>] [-jc <char>] [-dic <char>]
          [-training_type 0|1|2|3|4|5] [-kind_of_cityblock 0|1|2]
          [-t <int or float>] [-tfb <int or float>] [-tv <int or float>]
          [-substringCheck] [-noConsecutiveDelAndIns] [-noEqMap]
          [-alignment_type 0|1|2] [-doubtful_context_size <int>]
          [-doubtful_condition 0|1] [-t_nbest <int>] [-f_nbest <int>]
          [-s_nbest <int>] [-output_nbest <int>] [-nbest <int>] [-h]

オプション:
  -i <string>
   <string>にアライメントを取りたい2つの文字列が書かれたファイル名を
　　指定するオプションです．そのファイル名に書かれた対データをもとに学習を行い
　　ます．対データのフォーマットは2つの文字列をタブ(\t)区切りにし，各文字列の
　　文字間または部分文字列間に空白を入れたフォーマットでなければなりません．
　　部分文字列は学習によって文字に分割されることがなく，部分文字列を使うことで
　　アライメントに制約を与えることができます．以下にフォーマットの例を示します．

東 京	ト ウ キョ ウ
京 都	キョ ウ ト
i P h o n e	ア イ フォ ン
i P a d	ア イ パッ ド

　　また以下のように部分的にアライメントの正解を与えることが可能です．

東|京	ト ウ|キョ ウ
京 都	キョ:ウ ト
i P h o n|e	ア イ フォ ン|_
i P a d	ア:イ パッ ド

　　ここで，「_」は削除文字です．また，区切り文字「|」は表記と読みにおいて
　　同じ数でなければいけません．

  -o <string>
　　<string>に対データをアライメントした結果を出力するファイル名を指定します．
　　デフォルトは「-i」オプションで指定したファイル名に.alignの拡張子を付けた
　　ものになります．

  -p <string>
    <string>に対応付けが分かっている部分文字列のペアが書かれたファイル名を
　　指定します．事前知識を与えたい場合にこのオプションを使います．
　　フォーマットは以下の通りで，各要素はタブ(\t)区切りになっています．
　　1を指定するとその対応関係が選ばれるようになります．0を指定するとその
　　対応関係は決して選ばれません．

s:y	じー	1
田	だ	1
高:田	た:か:だ	0
e	_	0

  -ai <string>
　　<string>に学習したアライメントのパラメータが書かれたファイル名(デフォルトだと
    .modelの拡張子がついたファイル)を指定します．パラメータが書かれたファイルは
    -aoオプションを付けてmpalignerを動かすと出力されます．つまり，
　　このオプションは一度学習したパラメータでもう一度アライメントしたい場合や
　　新しいデータの中に今までにないパターンを持つデータがないか検出したい場合
　　などに使います．

  -ao <string>
    <string>に学習したパラメータを出力するファイル名を指定します．

  -output_used_mapping <string>
    一度でもアライメントに使われた対応付けだけを<string>に書き込みます．

  -s
　　アライメントした結果を出力するファイルにそのアライメントのスコアや詳細情報も
　　出力します．

  -h <int>
　　アライメントのパラメータを格納するハッシュサイズを指定します．
    デフォルトは99,940,009です．

  -del
    対データのX側（左側）が削除文字に対応付くことを許します．
　　例えば以下のようなアライメントを可能にします．

つ|の|だ|☆|ひ|ろ|	ツ|ノ|ダ|_|ヒ|ロ|

　　「_」は削除文字です．X側の「☆」が削除文字に対応していることが分かります．
　　つまり，この例では「☆」に読みは存在しないという対応付けになっています．
　　デフォルトではX側が削除文字に対応することを許していません．

  -ins
    挿入文字を許可します．(削除文字とは逆で，Y側（右側）に「_」の出現を許す．
　　デフォルトでは許可しません．)

  -dp <float>
　　削除文字の出現ペナルティです．高いほど削除文字が抑制されます．
　　このパラメータの値は経験的に決定しなければなりません．
　　だいたい0.1〜1.0の範囲で設定します．
　　(default 0.5)

  -ip <float>
    挿入文字の出現ペナルティです，(default 1.0)

  -rx <int>
    X側（左側）の部分文字列の最大文字数です.
    (default 0: 0は最大文字数の制約なしです．)

  -ry <int>
    Y側（左側）の部分文字列の最大文字数です.
    (default 0: 0は最大文字数の制約なしです．)

  -uc <char>
    アライメントが未知であることを示す記号です．(default ' ')

  -sc <char>
    アライメントが区切りであることを示す記号です．(default '|')

  -jc <char>
    アライメントが結合であることを示す記号です． (default ':')

  -dic <char>
    削除文字または挿入文字を示す記号です． (default '_')

  -training_type 0|1|2|3|4|5
    学習のタイプを示します. (defalut 5)

    0:市街地距離を導入しないforward-backward 学習です．

    1:forward-backward 学習を行った後，n-best viterbi training を行います．
　　　市街地距離は導入しません．

    2:市街地距離を導入する forward-backward 学習です．

    3:forward-backward 学習を行った後，n-best viterbi trainingを行います．
　　　それぞれにおいて市街地距離を導入します．

    4:削除文字と挿入文字を許可しない forward-backward 学習を行った後，
　　　それらを許可し，その文字の出現確率を周辺の対応関係から求める
　　　n-best viterbi training を一回だけ行います．
　　　それぞれにおいて市街地距離を導入します．

    5:削除文字と挿入文字を許可しない forward-backward 学習を行った後，
　　　それらを許可し，その文字の出現確率を周辺の対応関係から求める
　　　n-best viterbi training を一回だけ行います．
　　　その後，さらに n-best viterbi training で学習を行います．
　　　それぞれにおいて市街地距離を導入します．

  -t <int or float>
　　forward-backward 学習と n-best viterbi training の終了に関する閾値です．
　　もし，閾値が1以上ならばその回数だけ学習を繰り返します．もし，閾値が1未満なら
　　学習により変化したパラメータの合計変化値と閾値を比較して，閾値よりも合計変化
　　値が低ければ学習を終了します．(defalut 0.1)
　　forward-backward 学習と n-best viterbi training それぞれに対して閾値を設定
　　したい場合は-tfbと-tvのオプションを使用します．

  -kind_of_cityblock 0|1|2
    市街地距離の種類です．(defalut 0)

    0:X側（右側）とY側(左側）の合計文字数

    1:Y側(左側）の合計文字数

    2:X側(左側）の合計文字数

  -tfb <int or float>
    forward-backward 学習を終了する閾値です．(defalut 0.1)

  -tv <int or float>
    n-best viterbi training を終了する閾値です．(defalut 0.1)

  -substringCheck
    部分文字列かどうかを調べます．デフォルトでは調べません．
　　（特殊な機能なので使う機会はないでしょう．）

  -noConsecutiveDelAndIns
    削除文字と挿入文字の連続を禁止します．
　　例えば以下のようにアライメントされません．

G|r|e|e|e|n|	グ|リ|ー|_|_|ン|

  -noEqMap
    X側とY側が同じ文字数の場合は対応付けしません．

  -alignment_type 0|1|2
    alignment type. (defalut 1)

    0:誤った対応付けを結合しないアライメントを行います．

    1:誤った対応付けを結合するアライメントを行います．

    2:誤った対応付けを検出して出力ファイルである*.errに書きこみます．

  -doubtful_context_size <int>
　　誤った対応付けと判断するコンテキスト数を指定します．(defalut 1)

  -doubtful_condition 0|1
    誤った対応付けを判断する条件を指定します．(defalut 1)

    0:対応付けのコンテキスト数が前後とも小さい場合は誤った対応付けとして
　　　判断します．もし，doubtful_context_size=1 で対応付けの形式を
　　　<[context_size]mapping[context_size]>と表すなら,
 　   "<[9]mapping[1]><[1]mapping[5]>"は誤ったマッピングとなります．

    1:対応付けのコンテキスト数が前または後のどちらかで小さい場合は
　　　誤った対応付けとして判断します．もし doubtful_context_size=1 で対応
　　　付けの形式を<[context_size]mapping[context_size]>と表すなら,
  　  "<[9]mapping[4]><[1]mapping[5]>"と"<[9]mapping[1]><[3]mapping[5]>"は
　　　誤ったマッピングとなります．

  -t_nbset <int>
　　n-best viterbi training 時の学習するアライメント数です．(defalut 2)

  -f_nbest <int>
    最初のアライメントの n-best 数です．(defalut 1)
　　alignment type が1または2の時，このn-bestの結果に基づいて，
　　誤ったマッピングを発見します．0の時はこれが出力結果となります．

  -s_nbest <int>
    alignment type が1の時に有効になるオプションで2回目のアライメント時の
　　n-best 数です．(defalut 1)

  -output_nbest <int>
　　アライメントの出力結果数．(defalut -f_nbest or -s_nbest)

  -nbest <int>
　　n-best 指定のためのオプションです.
    もし alignment_type が0か2ならこのオプションは -f_nbest と同じです．
    もし alignment_type が1ならこのオプションは -s_nbest と同じです．
```

# バグの報告

もし何かバグを発見されましたら，以下のアドレスまでご報告いただけると幸いです．

keigokubo{＠}gmail.com  << {＠}を@に変換してください
