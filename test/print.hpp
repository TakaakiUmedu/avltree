// https://github.com/TakaakiUmedu/python_like_print

#ifndef PYTHON_LIKE_PRINT
#define PYTHON_LIKE_PRINT

/*** 推奨するincludeの仕方

// ↓ここから
//#include <atcoder/segtree>
//#include <atcoder/lazysegtree>
//#include <atcoder/fenwicktree>
//#include <atcoder/modint>
//#include <atcoder/dsu>

#include <string>
#include <vector>
#include <iostream>
#include <tuple>
#ifdef USE_PYTHON_LIKE_PRINT
#include "print.hpp"
#else
template<char SEP = ' ', char END = '\n', class... A> inline void print(A& ...){}
template<char SEP = ' ', char END = '\n', class... A> inline void printe(A& ...){}
template<char SEP = ' ', char END = '\n', class... A> inline void printo(A& ...){}
#define define_print(...)
#define define_print_with_names(...)
#define define_to_tuple(...)
#endif
// ↑ここまで

 ***/

#include <algorithm>

// Python風に標準出力に出力する。
// SEP、ENDを付けるとそのように出力される。
// SEPに''を指定するとエラーになる。指定したい場合には代わりにprint<'\0'>()とする。
// ENDを付けたい場合、SEPは省略できない。
template<char SEP = ' ', char END = '\n', typename ...A> inline void print(const A& ...args);

// 同じものの標準エラーバージョン
template<char SEP = ' ', char END = '\n', typename ...A> inline void printe(const A& ...args);

// 同じものの出力先自由バージョン
template<char SEP = ' ', char END = '\n', typename ...A> inline void printo(std::ostream& out, const A& ...args);

// classやstructの中身を ostream& への << で出力できるようにする。
// classやstructの ** 定義の中 ** で、
//
//    define_print( 表示させたいメンバをカンマ区切りで好きなだけ, ...)
//
// とすると、print(そのクラスのオブジェクト); ができるようになる(そのオブジェクトを含むvectorやmap等も同様)。
//
//    define_print_with_names(表示させたいメンバをカンマ区切りで好きなだけ, ...)
//
// とすると、それぞれ「メンバ名: 値」と表示されてより見やすくなる。
// 同じ関数を定義しているので、両方同時に指定するとエラー。
#define define_print(...) \
	inline void python_like_print(std::ostream& out) const{ \
		out << "{"; \
		python_like_print::print_members(out, __VA_ARGS__); \
		out << "}"; \
	}

#define define_print_with_names(...) \
	inline static std::vector<std::string>& python_like_print_member_names(){ \
		static std::vector<std::string> member_names = python_like_split_and_strip::split_and_strip(#__VA_ARGS__, ','); \
		return member_names; \
	}\
	inline void python_like_print(std::ostream& out) const{ \
		out << "{"; \
		python_like_print::print_members_with_names<0>(out, python_like_print_member_names(), __VA_ARGS__); \
		out << "}"; \
	}

// 同じようなノリで、指定したメンバーをtupleに変換する関数を定義する
#define define_to_tuple(...) \
	inline decltype(std::make_tuple(__VA_ARGS__)) to_tuple() const{\
		return (decltype(std::make_tuple(__VA_ARGS__)))(*this);\
	}\
	inline operator decltype(std::make_tuple(__VA_ARGS__))() const{\
		return std::make_tuple(__VA_ARGS__);\
	}


// 宣言を纏めて
namespace python_like_print{
	using std::string, std::basic_ostream, std::ostream, std::cout, std::endl;
	using std::pair, std::tuple, std::vector, std::tuple_size, std::get;
	using std::declval, std::enable_if_t, std::is_same_v, std::remove_reference_t, std::is_lvalue_reference_v, std::true_type, std::false_type;
	static bool print_char_as_int = false;
	
	// iteratorの類のように、begin()とend()が付いている場合に、*begin()の型を取得するためのクラス
	template<typename T,
	         typename B = decltype(declval<T>().begin()),
	         typename E = decltype(declval<T>().end()),
	         typename I = decltype(declval<B>().operator++()),
	         typename C = decltype(static_cast<bool>(declval<B>() != declval<E>())),
	         typename V = decltype(*declval<B>())>
	struct iterator_return_type{
		using type = enable_if_t<is_same_v<C, bool>, V>;
	};
	
	// pairかどうか判定
	template<typename T> struct is_pair: false_type{};
	template<typename F, typename S> struct is_pair<pair<F, S>>: true_type{};
	
	                                          inline void print_item(ostream& out, const string& value);
	                                          inline void print_item(ostream& out, const char* value);
	                                          inline void print_item(ostream& out, char value);
	template<typename First, typename Second> inline void print_item(ostream& out, const pair<First, Second>& pair);
	template<typename ...T>                   inline void print_item(ostream& out, const tuple<T...>& tuple);
	                                          inline void print_item(ostream& out, bool value);
	template<typename T, typename V = typename iterator_return_type<T>::type>
	                                          inline void print_item(ostream& out, const T& t);
	template<typename T, typename S = decltype(cout << declval<T>()), typename R = enable_if_t<is_same_v<remove_reference_t<S>, basic_ostream<char>> && is_lvalue_reference_v<S>>>
	                                          inline void print_item(ostream& out, const T& value);
	
	template<typename T, typename X = decltype(declval<T>().python_like_print(cout))>
	                                          inline ostream& operator<<(ostream& out, const T& value);
	
	template<typename IteratorB, typename IteratorE> inline void print_all(ostream& out, IteratorB begin, IteratorE end);
	template<size_t N = 0, typename ...T> inline void print_items_in_tuple(ostream &out, const tuple<T...>& tuple);
	template<size_t N, typename T> inline void print_members_with_names(ostream& out, vector<string> names, const T& arg);
	template<size_t N, typename F, typename ...T> inline void print_members_with_names(ostream& out, vector<string> names, const F& arg, T ...args);
	
	template<char SEP, char END>                            inline void print_to_stream(ostream& out);
	template<char SEP, char END, typename T, typename ...A> inline void print_to_stream(ostream& out, const T& value, const A& ...args);
}

// 標準出力に出力する関数の実体
template<char SEP = ' ', char END = '\n', typename ...A> inline void print(const A& ...args){
	printo<SEP, END, A...>(std::cout, args...);
}

// 同じものの標準エラーバージョン
template<char SEP = ' ', char END = '\n', typename ...A> inline void printe(const A& ...args){
	printo<SEP, END, A...>(std::cerr, args...);
}

// 同じものの出力先自由バージョン
template<char SEP = ' ', char END = '\n', typename ...A> inline void printo(std::ostream& out, const A& ...args){
	python_like_print::print_to_stream<SEP, END, A...>(out, args...);
}

// AtCoderライブラリのクラスにget(index)メソッドや、to_vector()メソッドを生やし、print()で表示できるようにするための拡張・置き換え用クラス
namespace python_like_print{
	// get(index)を生やす
	template<class S, class C, S _get(C&, int)> class gettable_collection: public C{
		public:
			gettable_collection(): C(){
			}
			gettable_collection(int size): C(size){
			}
			S get(int index){
				return _get(*this, index);
			}
	};
	
	// to_vector()のため、size()を生やす(_nがprivateで外からアクセス出来ないため)
	template<class S, class C> class vectorizable_collection: public C{
		private:
			int _size;
		public:
			vectorizable_collection(): C(), _size(0){
			}
			vectorizable_collection(int size): C(size), _size(size){
			}
			inline vector<S> to_vector() const{
				// AtCoderライブラリの値を取得するメソッドが非constな場合があるので、コピーを作ってからアクセスする必要がある(非効率)
				C collection_tmp(*this);
				vector<S> vec(_size);
				for(int i = 0; i < _size; i ++){
					vec[i] = collection_tmp.get(i);
				}
				return vec;
			}
			template<class T2, class C2> friend ostream& operator<<(ostream& out, vectorizable_collection<T2, C2> value);
	};
	template<class S, class C> inline ostream& operator<<(ostream& out, vectorizable_collection<S, C> value){
		print_item(out, value.to_vector());
		return out;
	}
}

// AtCoderライブラリのFenwick Tree、Lazy Segment Treee、Segment Treeeをprint()で表示できるようにする
// そのままでは無理なので、vectorizableなバージョンを定義して、defineで置き換えてしまう
namespace atcoder{
#ifdef ATCODER_FENWICKTREE_HPP
	template<class S> get_fenwick_tree_value(fenwick_tree<S>& ft, int index){
		return ft.sum(index, index + 1);
	}
	template<class S> using fenwick_tree_vectorizable = python_like_print::vectorizable_collection<S, python_like_print::gettable_collection<S, fenwick_tree<S>, get_fenwick_tree_value<S>>>;
#define fenwick_tree fenwick_tree_vectorizable
#endif
#ifdef ATCODER_LAZYSEGTREE_HPP
	template <class S, S (*op)(S, S), S (*e)(), class F, S (*mapping)(F, S), F (*composition)(F, F), F (*id)()>
		using lazy_segtree_vectorizable = python_like_print::vectorizable_collection<S, lazy_segtree<S, op, e, F, mapping, composition, id>>;
#define lazy_segtree lazy_segtree_vectorizable
#endif
#ifdef ATCODER_SEGTREE_HPP
	template <class S, S (*op)(S, S), S (*e)()> using segtree_vectorizable = python_like_print::vectorizable_collection<S, segtree<int, op, e>>;
#define segtree segtree_vectorizable
#endif
}
// ModIntやそのvectorなどをprint()できるようにする。
namespace python_like_print{
#ifdef ATCODER_MODINT_HPP
	template<int m> inline ostream& operator<<(ostream& out, const atcoder::static_modint<m>& value){
		return out << value.val();
	}
	template<int id> inline ostream& operator<<(ostream& out, const atcoder::dynamic_modint<id>& value){
		return out << value.val();
	}
#endif
#ifdef ATCODER_DSU_HPP
	inline ostream& operator<<(ostream& out, const atcoder::dsu& value){
		print_item(out, atcoder::dsu(value).groups());
		return out;
	}
#endif
}


// 実装
namespace python_like_print{
	// 各種の値をそれらしく出力
	
	// 文字列は""で囲む
	inline void print_item(ostream& out, const string& value){
		out << '"' << value << '"';
	}
	inline void print_item(ostream& out, const char* value){
		out << '"' << value << '"';
	}
	
	// charは''で囲む。あるいは整数値として出力
	inline void print_item(ostream& out, char value){
		if(print_char_as_int){
			out << static_cast<int>(value);
		}else{
			out << '\'' << value << '\'';
		}
	}
	
	// pairは、:で区切って
	template<typename First, typename Second> inline void print_item(ostream& out, const pair<First, Second>& pair){
		print_item(out, pair.first);
		out << ": ";
		print_item(out, pair.second);
	}

// tupleの中身を出力
// constexprはC++17以降でないと使えないので、それ未満のバージョンでコンパイルされた場合は定義を飛ばす。
#if defined(__cplusplus) && __cplusplus >= 201703L
	// 2個目以降の要素を", "を付けて出力
	template<size_t N = 0, typename ...T> inline void print_items_in_tuple(ostream &out, const tuple<T...>& item){
		if constexpr(N < tuple_size<tuple<T...>>::value){
			out << ", ";
			print_item(out, get<N>(item));
			print_items_in_tuple<N + 1, T...>(out, item);
		}
	}
	
	// tupleは中身を", "で区切って'('～')'で囲って順に出力
	template<typename ...T> inline void print_item(ostream &out, const tuple<T...>& item){
		out << '(';
		if constexpr(tuple_size<tuple<T...>>::value > 0){
			print_item(out, get<0>(item));
			print_items_in_tuple<1, T...>(out, item);
		}
		out << ')';
	}
#endif
	
	// boolは"true"/"false"を出力
	inline void print_item(ostream& out, bool value){
		out << (value ? "true" : "false");
	}
	
	// .begin()と.end()が付いているものは順に表示。中身がpairの場合はmapの類と見做して{}で、それ以外は[]で囲む
	template<typename T, typename V = typename iterator_return_type<T>::type>
	inline void print_item(ostream& out, const T& iterable){
		if constexpr(is_pair<V>::value){
			out << '{';
		}else{
			out << '[';
		}
		print_all(out, iterable.begin(), iterable.end());
		if constexpr(is_pair<V>::value){
			out << '}';
		}else{
			out << ']';
		}
	}
	
	// その他、<< で出力できるもの(できないものはエラー)
	template<typename T, typename S = decltype(cout << declval<T>()), typename R = enable_if_t<is_same_v<remove_reference_t<S>, basic_ostream<char>> && is_lvalue_reference_v<S>>>
	inline void print_item(ostream& out, const T& value){
		out << value;
	}
	
	// pytho_like_print(ostream& out); メソッドが生えているものを out << item; で出力できるように
	template<typename T, typename X = decltype(declval<T>().python_like_print(cout))>inline ostream& operator<<(ostream& out, const T& value){
		value.python_like_print(out);
		return out;
	}
	
	// .begin()から.end()までを", "区切りで出力する
	template<typename IteratorB, typename IteratorE> inline void print_all(ostream& out, IteratorB begin, IteratorE end){
		if(begin != end){
			auto i = begin;
			print_item(out, *i);
			for(++ i; i != end; ++ i){
				out << ", ";
				print_item(out, *i);
			}
		}
	}
	
	// 引数1つ1つをばらけさせる
	// ばらけさせる残りが無ければENDだけ出力
	template<char SEP, char END> inline void print_to_stream(ostream& out){
		if(END != '\0'){
			out << END;
		}
	}
	// 2つ以上の引数がある場合は1つ目を出力してSEPを出力後、残りを出力
	template<char SEP, char END, typename T, typename ...A> inline void print_to_stream(ostream& out, const T& value, const A& ...args){
		print_item(out, value);
		if constexpr(sizeof...(A) > 0){
			if(SEP != '\0'){
				out << SEP;
			}
			print_to_stream<SEP, END, A...>(out, args...);
		}else{
			print_to_stream<SEP, END>(out);
		}
	}
	
	// classやstructの中身を表示するためのヘルパ関数
	template<typename T> inline void print_members(ostream& out, const T& arg){
		print_item(out, arg);
	}
	
	template<typename F, typename ...T> inline void print_members(ostream& out, const F& arg, T ...args){
		print_item(out, arg);
		out << ", ";
		print_members(out, args...);
	}
	
	template<size_t N, typename T> inline void print_members_with_names(ostream& out, vector<string> names, const T& arg){
		out << names[N] << ": ";
		print_item(out, arg);
	}
	
	template<size_t N, typename F, typename ...T> inline void print_members_with_names(ostream& out, vector<string> names, const F& arg, T ...args){
		out << names[N] << ": ";
		print_item(out, arg);
		out << ", ";
		print_members_with_names<N + 1>(out, names, args...);
	}
	
}

// splitとstripの実装
//
// ソースファイルを分割コンパイルする場合、
// - static vector<string> split(const string& str, const string& delim)
// - static string strip(const string& str)
// の2つは、このままだと関数が重複してリンクされるのを避けると、バイナリサイズが減るかも?
//
namespace python_like_split_and_strip{
	using namespace std;
	
	// split(文字列, 区切り) あれこれ
	                                 static vector<string> split(const string& str, const string& delim);
	template<typename S, typename T> inline vector<string> split(const S& str, const T& delim);
	template<typename S>             inline vector<string> split(const S& str, const char delim);
	
	// strip(文字列)
	static string strip(const string& str);
	
	// splitしてからそれぞれをstripする
	template<typename S, typename T> vector<string> split_strip(const S& str, const T& delim);
	

	// stringの代わりにchar*やcharも渡せるように
	template<typename S, typename T> vector<string> split(const S& str, const T& delim){
		return split(string(str), string(delim));
	}
	template<typename S> vector<string> split(const S& str, const char delim){
		return split(string(str), string(1, delim));
	}
	
	// splitした上でそれぞれをstripする
	template<typename S, typename T> vector<string> split_and_strip(const S& str, const T& delim){
		auto result = split(str, delim);
		transform(result.begin(), result.end(), result.begin(), strip);
		return result;
	}
	
	static vector<string> split(const string& str, const string& delim){
		vector<string> result;
		size_t start = 0;
		while(start < str.size()){
			auto index = str.find(delim, start);
			if(index == string::npos){
				break;
			}
			result.push_back(str.substr(start, index - start));
			start = index + delim.size();
		}
		result.push_back(str.substr(start));
		return result;
	}
	static string strip(const string& str){
		if(str.size() == 0){
			return str;
		}else{
			const auto start = str.find_first_not_of(" \t\r\n");
			if(start == string::npos){
				return string("");
			}else{
				const auto end = str.find_last_not_of(" \t\r\n");
				return str.substr(start, end - start + 1);
			}
		}
	}
}


#endif
