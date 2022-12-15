# avltree
C++ implementation of AVL Tree. O(log N) index access and segment tree like function available

[avltree::map](#avltreemapk-v-s-u)、[avltree::set](#avltreesetv-s-u)、[avltree::multiset](#avltreemultisetv-s-u)を提供。
操作は要素数Nに対してO(log N)か、O(1)。テンプレート引数の指定により以下のギミックをON/OFFできる。
 - O(log N)でのインデックスアクセス
 - 各部分木の高さを記録しておく(使い道が思いつかないけど試しに実装)
 - segment tree同様に、可換則が成り立つ任意の演算での区間集計をO(log N)で実行

## 共通仕様
 - 仕様
   - インデックスなどは0ベース
 - テンプレート引数
   - typename K: キーの型(mapのみ)
   - typename V: 値の型
   - [tree_spec](#avltreetree_spec) S: 追加のギミックを指定
   - typename U: [区間集計](#区間集計)を指定
 - メンバー関数(以下、xは、mapの場合は型がKのキー、setとmultisetでは型がVの値)
   - remove(x)
     - xを削除して削除した要素の[node_uptr_view](#node_uptr_view)を返す
   - pop_first()、pop_last()
     - それぞれ、最初、最後の要素を削除して削除した要素の[node_uptr_view](#node_uptr_view)を返す
   - find(x)
     - xの[node_view](#node_view)を返す
   - find_lt(x)、find_le(x)、find_ge(x)、find_gt(x)
     - それぞれ、xに最も近い、x未満、x以下、x以上、xより大きい、[node_view](#node_view)を返す
   - iterator_find(x)
     - xの[iterator](#iterator)を返す
   - contains(x)
     - xが含まれているかどうかをboolで返す
   - begin()
     - xが最小の[iterator](#iterator)を返す
   - end()
     - nullptrを返す(最後までiterateが終わった[iterator](#iterator)は==nullptrになるため)
   - rbegin()
     - kが最大の[reverse_iterator](#reverse_iterator)を返す
   - rend()
     - end()と同様
 - 追加のメンバー関数([tree_spec](#avltreetree_spec)::with_indexを指定した場合のみ利用可能)
   - at(size_t i)
     - i番目の要素の[node_view](#node_view)を返す
   - index(x)
     - xの出現インデックスを求める
   - pop_at(size_t i)
     - i番目の要素を削除して取り出し、[node_uptr_view](#node_uptr_view)を返す
   - iterator_at(size_t i)
     - i番目の要素の[iterator](#iterator)を返す
   - find_with_index(x)、find_lt_with_index(x)、find_le_with_index(x)、find_ge_with_index(x)、find_gt_with_index(x)
     - それぞれ「\_with_index」無しのものと同様の[node_view](#node_view)とそのインデックスを格納したstd::tupleを返す
 - 追加のメンバー関数(Uに[avltree::summerizer::single](#区間集計)を1個以上含むstd::tupleを指定した場合のみ利用可能)
   - summerize(x_s, x_e)
     - キー(mapの場合)もしくは値(set、multisetの場合)が、x_s以上、x_e*以下*の要素の集計結果を返す
 - 追加のメンバー関数([tree_spec](#avltreetree_spec)::with_indexを指定し、Uに[avltree::summerizer::single](#区間集計)を1個以上含むstd::tupleを指定した場合のみ利用可能)
   - summerize_by_index(i_s, i_e)   
     - i_s番目以上、i_e番目*未満*の要素の集計結果を返す
 - 型
   - ::data_type
     - mapの場合はpair<K, V>、setとmultisetの場合はV
   - [::node_view](#node_view)
   - [::node_uptr_view](#node_uptr_view)
   - [::iterator](#iterator)
   - [::reverse_iterator](#reverse_iterator)
   - [::with_summary<typename X, X summarize(const X& a, const X& b), X identity(), X get(const data_type& d) = summarizer::pass>](#区間集計)
     - 関数で指定した区間集計を追加した型
     - 複数追加可能で、複数追加した場合は区間集計の返り値は集計結果それぞれを含むstd::tupleになる
   - [::with_summary_key_sum](#区間集計)、[::with_summary_key_prod](#区間集計)、[::with_summary_value_sum](#区間集計)、[::with_summary_value_prod](#区間集計)、[::with_summary_value_min](#区間集計)、[::with_summary_value_max](#区間集計)
     - mapで利用可能
     - 汎用の::with_summary<>の単純なものを簡単に指定できるようにしたショートカット
     - それぞれ、キーの合計、積算、値の合計、積算、最小、最大の区間集計を追加した型
   - [::with_summary_sum](#区間集計)、[::with_summary_prod](#区間集計)
     - set、multisetで利用可能
     - 汎用の::with_summary<>の単純なものを簡単に指定できるようにしたショートカット
     - それぞれ、合計、積算を追加した型

## avltree::map<K, V, S, U>
 - 連想配列
 - コンストラクタ
   - avltree::map<K, V, S>()
     - 空の連想配列を作る
   - avltree::map<K, V, S>(b, e)
     - pair<K, V>やtuple<K, V>などを返すbegin()からend()で指定された要素で初期化
 - メンバー関数
   - insert(k, v)
     - 値vをキーkに関連付けて記録
     - kが記録されていない場合は新規に記録してtrueを、既にkが記録されている場合はvで上書きしてfalseを返す

## avltree::set<V, S, U>
 - 集合
 - コンストラクタ
   - avltree::set<V, S, U>()
     - 空の集合を作る
   - avltree::set<V, S, U>(b, e)
     - Vを返すbegin()からend()で指定された要素で初期化
 - メンバー関数
   - insert(v)
     - 値vを集合に追加
     - 追加された場合はtrue、既に含まれている値で追加されなかった場合はfalseを返す

## avltree::multiset<V, S, U>
 - 多重集合
 - コンストラクタ
   - avltree::multiset<V, S, U>()
     - 空の集合を作る
   - avltree::multiset<V, S, U>(b, e)
     - Vを返すbegin()からend()で指定された要素で初期化
 - メンバー関数
   - insert(v)
     - 値vを集合に追加
     - 常にtrueを返す

## avltree::tree_spec
 - 下記の値を|で繋いで指定可能(by_refとby_valの両方を同時に指定するとエラー)
   - avltree::tree_spec::simple            : 追加のギミック無し
   - avltree::tree_spec::with_index        : O(log N)のインデックスアクセス機能を付ける
   - avltree::tree_spec::with_depth        : 各ノードはそのノードを頂点とする部分木の高さを持つ
   - avltree::tree_spec::pass_key_by_ref   : 各メンバメソッドの引数のkをconst参照渡しにする
   - avltree::tree_spec::pass_key_by_val   : 各メンバメソッドの引数のkを値渡しにする
   - avltree::tree_spec::pass_value_by_ref : 各メンバメソッドの引数のvをconst参照渡しにする
   - avltree::tree_spec::pass_value_by_val : 各メンバメソッドの引数のvを値渡しにする

## node_view
 - 検索結果を返す
 - data_typeへのポインタ風に使えるoperatorを提供
 - nullptrとの比較でtrueになる場合や、条件分岐などでのboolへの変換でfalseになる場合は、対象の要素は存在しない
 - 内容はデータ構造に変更が行われるまでは利用可能
 - メンバー関数
   - size()  (with_indexを付与した場合のみ利用可能)
     - 部分木のノード数を返す

## node_uptr_view
 - node_viewと同じだが、データ構造から既に消された要素へのunique_ptrを含む。そのため、コピーは不可
 - 内容は、返されたnode_uptr_viewか、その内容を移動した先の変数の寿命が尽きるまでは利用可能

## iterator
 - node_viewにiteratorの機能を追加したもの

## reverse_iterator
 - iteratorと逆順でiterateするiterator

## 区間集計
 - テンプレート引数のUには、複数のavltree::summarizer::single<>を含むstd::tupleを指定する
 - Uが空の場合は区間集計機能はOFFになる
 - avltree::summarizer::single<typename X, X summarize(const X& a, const X& b), X identity(), X get(const data_type& d) = summarizer::pass>
   - X: 集計結果の型
   - summarize: Xの値2つを1つに集計する関数
   - identity: Xの単位元、すなわち、x == summarize(x, identity())となるような値を返す関数。summerize(a, b)がa+bなら0、a\*bなら1など
   - get: 各ノードのdata_typeから集計対象とする値をXに変換する関数を指定する。mapの場合はキー(d.first)と値(d.second)のどちらを集計するかなど。Xとdata_typeが等しい場合は省略可能で、省略した場合はdata_typeがそのまま集計対象になる
 - Uが1個の要素のみを場合は、summarize()やsummarize_by_index()は、集計結果をXの値として返す
 - Uが複数の要素を含む場合は、summarize()やsummarize_by_index()は、それぞれの集計結果を同じ順序でstd::tupleで束ねた値として返す

## 利用例
- インデックスアクセスできて、区間の合計値を求められる多重集合
```
// tree_spec::with_indexを指定したint64_t型の多重集合に、with_summary_sumで区間の合計値を集計する機能を追加
avltree::multiset<int64_t, avltree::tree_spec::with_index>::with_summary_sum values();
// 先頭K個の値の合計値を出力
std::cout << values.summarize_by_index(0, K) << std::endl;
```

- 区間の値の積と、区間が含む値全てが入ったstd::vector<>を求められる集合(※vectorは、そういうこともできるというデモのためのデモで、実用性は無い)
```
// vectorを繋ぐ
std::vector<int64_t> summarize(const std::vector<int64_t>& a, const std::vector<int64_t>& b){
	std::vector<int64_t> c(a.begin(), a.end());
	std::copy(b.begin(), b.end(), std::back_inserter(c));
	return c;
}
// 空のvectorを返す
std::vector<int64_t> identity(){
	return std::vector<int64_t>();
}
// 要素数が1個のvectorを返す
std::vector<int64_t> get(const int64_t& d){
	return std::vector<int64_t>(1, d);
}
// int64_t型の集合に、with_summary_prodで区間の積を集計する機能と、with_summary<>で上記の3つの関数を使って値をvectorに集める機能を追加
avltree::set<int64_t>::::with_summary_prod::with_summary<std::vector<int64_t>, summarize, identity, get> values();
// 集合に含まれるs以上t以下の値について集計
tuple<int64_t, vector<int64_t>> result = values.summarize(s, t);
// 集合に含まれるs以上t以下の値の積を表示
std::cout << get<0>(result) << std::endl;
// 集合に含まれるs以上t以下の値を順に表示
for(const auto v: get<1>(result)){
	std::cout << v << std::endl;
}
```