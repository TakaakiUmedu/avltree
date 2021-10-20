#include <set>
#include <map>
#include <vector>
#include <random>

class checker;
#define AVLTREE_DEBUG_CLASS ::checker

#include "../avltree.hpp"

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


using namespace std;

class checker{
public:
	template<typename K, typename V, avltree::tree_spec S = 0> static tuple<size_t, size_t, bool> check_node(const typename avltree::avltree_base::avltree<K, V, S>::node& n){
		bool failed = false;
		auto [dl, cl, fl] = n.l ? check_node<K, V, S>(*n.l) : (tuple<size_t, size_t, bool>)make_tuple(0, 0, false);
		auto [dr, cr, fr] = n.r ? check_node<K, V, S>(*n.r) : (tuple<size_t, size_t, bool>)make_tuple(0, 0, false);
		int b = n.balance();
		
		if(b != dl - dr || (b != -1 && b != 0 && b != 1)){
			cout << "unbalance: " << /*n._k << */ ", " << b << " <-> (" << dl << ", " << dr << ")" << endl;
			failed = true;
		}
		
		const auto count = cl + cr + 1;
		
		if constexpr(avltree::avltree_base::avltree<K, V, S>::with_index::value){
			if(count != n.c){
				cout << "count mismatch:: " << n.c << " <-> " << count << endl;
				failed = true;
			}
		}
		
		return make_tuple(max(dl, dr) + 1, count, fl || fr || failed);
	}
	template<typename T> static bool check_tree(T& t){
		using K = typename T::K_;
		using V = typename T::V_;
		const auto S = T::S_;
		if(t.root){
			bool failed = false;
			auto [d, c, f] = check_node<K, V, S>(*t.root);
			if(c != t.size()){
				cout << "count mismatch: " << c << " <-> " << t.size() << endl;
				failed = true;
			}
			return !(f || failed);
		}else{
			return true;
		}
	}
	
	template<typename T, typename M> static void test(const int64_t N){
		using K = typename T::K_;
		using V = typename T::V_;
		const auto S = T::S_;
		using is_multiset = std::is_same<T, avltree::multiset<K, S>>;
		std::uniform_int_distribution<> coin(0, 2);
		std::uniform_int_distribution<> dist(0, N);
		std::random_device seed_gen;
//		std::mt19937 engine(seed_gen());
		std::mt19937 engine(0);
		{
			T tree;
			M std_tree;
			
	//		std::map<int64_t, int64_t> std_tree;
			
			for(int i = 0; i < N; i ++){
				const int64_t x = dist(engine);
				if(coin(engine) == 0 && std_tree.find(x) != std_tree.end()){
					std_tree.erase(std_tree.find(x));
					tree.remove(x);
					if(!check_tree(tree)){
						cout << "remove error: " << x << endl;
						tree.print();
						throw "ERROR";
					}
				}else{
					const int64_t y = dist(engine);
					if constexpr(T::with_value::value){
						tree.insert(x, y);
						std_tree[x] = y;
					}else{
						tree.insert(x);
						std_tree.insert(x);
					}
					if(!check_tree(tree)){
						cout << "insert error" << endl;
						tree.print();
						throw "ERROR";
					}
				}
				for(int j = 0; j < N; j ++){
					if(tree.contains(j) != (std_tree.find(j) != std_tree.end())){
						cout << "set error:" << j << endl;
						cout << tree.contains(j) << endl;
						cout << (std_tree.find(j) != std_tree.end()) << endl;
						throw "ERROR";
					}
				}
				if constexpr(T::with_index::value){
					size_t j = 0;
					if constexpr(T::with_value::value){
						for(auto& [k, v] : std_tree){
							if(tree.at(j)->first != k){
								std::cout << "index error" << std::endl;
								throw "ERROR";
							}
							if(tree.at(j)->second != v){
								std::cout << "index error" << std::endl;
								throw "ERROR";
							}
							j ++;
						}
					}else{
						for(auto& k : std_tree){
							if(*tree.at(j) != k){
								std::cout << "index error" << std::endl;
								throw "ERROR";
							}
							j ++;
						}
					}
				}
				auto i_b = std_tree.begin();
				auto i_e = std_tree.end();
				if constexpr(T::with_value::value){
					for(auto [k, v] : tree){
//						print(k, v, i_b->first);
						if(!(i_b != i_e) || i_b->first != k || i_b->second != v){
							print(std_tree);
							cout << "iter error " << (i_b != i_e) << " " << std_tree.size() << " " << (std_tree.begin() != std_tree.end()) << " " << (i_b->first != k) << " " << i_b->first << "," << k << endl;
							throw "ERROR";
						}
						++i_b;
					}
					i_b = std_tree.begin();
					for(auto k : tree.keys()){
//						print(k, v, i_b->first);
						if(!(i_b != i_e) || i_b->first != k){
							print(std_tree);
							cout << "iter error " << (i_b != i_e) << " " << std_tree.size() << " " << (std_tree.begin() != std_tree.end()) << " " << (i_b->first != k) << " " << i_b->first << "," << k << endl;
							throw "ERROR";
						}
						++i_b;
					}
					i_b = std_tree.begin();
					for(auto v : tree.values()){
//						print(k, v, i_b->first);
						if(!(i_b != i_e) || i_b->second != v){
							print(std_tree);
							cout << "iter error " << (i_b != i_e) << " " << std_tree.size() << " " << (std_tree.begin() != std_tree.end()) << " "  << i_b->first << "," << endl;
							throw "ERROR";
						}
						++i_b;
					}
				}else{
					for(auto k : tree){
//						print(k, *i_b);
						if(!(i_b != i_e) || *i_b != k){
							print(std_tree);
							cout << "iter error " << (i_b != i_e) << " " << std_tree.size() << " " << (std_tree.begin() != std_tree.end()) << " " << (*i_b != k) << " " << *i_b << "," << k << endl;
							throw "ERROR";
						}
						++i_b;
					}
				}
				if(i_b != i_e){
					print(std_tree);
					print(tree);
					tree.print();
					cout << "iter end error" << endl;
					throw "ERROR";
				}
				if constexpr(T::with_index::value){
					size_t i = 0;
					for(i_b = std_tree.begin(); i_b != std_tree.end(); i_b ++){
						if constexpr(T::with_value::value){
							if(!tree.at(i) || tree.at(i)->first != i_b->first){
								cout << "index error 00" << endl;
								throw "ERROR";
							}
							if constexpr(is_multiset::value){
								size_t l = tree.index(i_b->first);
								size_t r = l + tree.count(i_b->first);
								if(i < l || i >= r){
									print(l, i, r);
									cout << "index error 01" << endl;
									throw "ERROR";
								}
							}else{
								if(tree.index(i_b->first) != i){
									cout << "index error 02" << endl;
									throw "ERROR";
								}
							}
						}else{
							if(!tree.at(i) || *tree.at(i) != *i_b){
								cout << "index error 03" << endl;
								throw "ERROR";
							}
							if constexpr(is_multiset::value){
								size_t l = tree.index(*i_b);
								size_t r = l + tree.count(*i_b);
								if(i < l || i >= r){
									print(*i_b, l, i, r);
									print(tree);
									print(std_tree);
									cout << "index error 04" << endl;
									throw "ERROR";
								}
							}else{
								if(tree.index(*i_b) != i){
									cout << "index error 05" << endl;
									throw "ERROR";
								}
							}
						}
						i ++;
					}
					if(tree.at(-1) || tree.at(std_tree.size())){
						cout << "index error" << endl;
						throw "ERROR";
					}
				}
			}
			print("r", tree.size());
			
			
			{
				const std::tuple<int64_t, int64_t> min_value = std::make_tuple(-1, 0);
				const std::tuple<int64_t, int64_t> max_value = std::make_tuple(N + 1, 0);
				for(int64_t i = -1; i <= N; i ++){
					auto s_ub = std_tree.upper_bound(i);
					auto s_lb = std_tree.lower_bound(i);
					auto t_gt = tree.find_gt(i);
					auto t_ge = tree.find_ge(i);
					auto t_le = tree.find_le(i);
					auto t_lt = tree.find_lt(i);
					
					int64_t k_s_ub;
					int64_t k_s_lb;
					int64_t k_s_pv = -1;
					
					if constexpr(T::with_value::value){
						k_s_ub = get<0>(s_ub != std_tree.end() ? *s_ub : max_value);
						k_s_lb = get<0>(s_lb != std_tree.end() ? *s_lb : max_value);
					}else{
						k_s_ub = s_ub != std_tree.end() ? *s_ub : get<0>(max_value);
						k_s_lb = s_lb != std_tree.end() ? *s_lb : get<0>(max_value);
					}
					if(s_lb != std_tree.begin()){
						s_lb --;
						if constexpr(T::with_value::value){
							k_s_pv = s_lb->first;
						}else{
							k_s_pv = *s_lb;
						}
					}
					int64_t k_t_gt;
					int64_t k_t_ge;
					int64_t k_t_le;
					int64_t k_t_lt;
					
					if constexpr(T::with_value::value){
						k_t_gt = std::get<0>(t_gt ? *t_gt : max_value);
						k_t_ge = std::get<0>(t_ge ? *t_ge : max_value);
						k_t_le = std::get<0>(t_le ? *t_le : min_value);
						k_t_lt = std::get<0>(t_lt ? *t_lt : min_value);
					}else{
						k_t_gt = t_gt ? *t_gt : std::get<0>(max_value);
						k_t_ge = t_ge ? *t_ge : std::get<0>(max_value);
						k_t_le = t_le ? *t_le : std::get<0>(min_value);
						k_t_lt = t_lt ? *t_lt : std::get<0>(min_value);
					}
					
	//				print(k_t_lt, k_t_le, k_t_ge, k_t_gt);
					if(std_tree.find(i) != std_tree.end()){
						if(!t_ge || !t_le){
							cout << "find bound error1" << !t_ge << ", " << !t_le << endl;
							throw "ERROR";
						}
						
						if(k_s_lb != k_t_ge || k_s_lb != k_t_le){
							cout << "find bound error2" << endl;
							throw "ERROR";
						}
						if(k_t_gt <= k_t_ge || k_t_lt >= k_t_le || k_t_gt != k_s_ub || k_t_lt != k_s_pv){
							cout << "find bound error3" << endl;
							throw "ERROR";
						}
					}else{
						if(k_t_gt != k_t_ge || k_t_lt != k_t_le){
							cout << "find bound error4" << endl;
							throw "ERROR";
						}
						if(k_t_gt != k_s_ub || k_t_lt != k_s_pv){
							print(k_t_gt, k_s_ub, k_t_lt, k_s_pv);
							cout << "find bound error5" << endl;
							throw "ERROR";
						}
					}
					if constexpr(T::with_index::value){
						auto [d_t_gt, i_t_gt] = tree.find_gt_with_index(i);
						auto [d_t_ge, i_t_ge] = tree.find_ge_with_index(i);
						auto [d_t_le, i_t_le] = tree.find_le_with_index(i);
						auto [d_t_lt, i_t_lt] = tree.find_lt_with_index(i);
						int64_t d_k_t_gt;
						int64_t d_k_t_ge;
						int64_t d_k_t_le;
						int64_t d_k_t_lt;
						
						if constexpr(T::with_value::value){
							d_k_t_gt = std::get<0>(d_t_gt ? *d_t_gt : max_value);
							d_k_t_ge = std::get<0>(d_t_ge ? *d_t_ge : max_value);
							d_k_t_le = std::get<0>(d_t_le ? *d_t_le : min_value);
							d_k_t_lt = std::get<0>(d_t_lt ? *d_t_lt : min_value);
						}else{
							d_k_t_gt = d_t_gt ? *d_t_gt : std::get<0>(max_value);
							d_k_t_ge = d_t_ge ? *d_t_ge : std::get<0>(max_value);
							d_k_t_le = d_t_le ? *d_t_le : std::get<0>(min_value);
							d_k_t_lt = d_t_lt ? *d_t_lt : std::get<0>(min_value);
						}
						if(k_t_gt != d_k_t_gt || i_t_gt != tree.index(k_t_gt)){
							print(k_t_gt, d_k_t_gt, i_t_gt, tree.index(k_t_gt));
							print(std_tree);
							print(tree);
							cout << "find bound error6" << endl;
							throw "ERROR";
						}
						if(k_t_ge != d_k_t_ge || i_t_ge != tree.index(k_t_ge)){
							print(k_t_ge, d_k_t_ge, i_t_ge, tree.index(k_t_ge));
							print(std_tree);
							print(tree);
							cout << "find bound error7" << endl;
							throw "ERROR";
						}
						if constexpr(is_multiset::value){
							if(k_t_le != d_k_t_le || i_t_le != tree.last_index(k_t_le)){
								print(k_t_le, d_k_t_le, i_t_le, tree.last_index(k_t_le));
								print(std_tree);
								print(tree);
								cout << "find bound error8" << endl;
								throw "ERROR";
							}
							if(k_t_lt != d_k_t_lt || i_t_lt != tree.last_index(k_t_lt)){
								print(std_tree);
								print(tree);
								cout << "find bound error9" << endl;
								throw "ERROR";
							}
						}else{
							if(k_t_le != d_k_t_le || i_t_le != tree.index(k_t_le)){
								print(k_t_le, d_k_t_le, i_t_le, tree.index(k_t_le));
								print(std_tree);
								print(tree);
								cout << "find bound error10" << endl;
								throw "ERROR";
							}
							if(k_t_lt != d_k_t_lt || i_t_lt != tree.index(k_t_lt)){
								print(std_tree);
								print(tree);
								cout << "find bound error11" << endl;
								throw "ERROR";
							}
						}
					}
				}
			}
		}
		{
			T tree;
			std::vector<typename T::K_> data;
			for(int i = 0; i < 20; i ++){
				int64_t x = dist(engine);
				while(tree.contains(x)){
					x = dist(engine);
				}
				data.push_back(x);
				if constexpr(T::with_value::value){
					tree.insert(x, x);
				}else{
					tree.insert(x);
				}
			}
			std::sort(data.begin(), data.end());
			for(int j = 0; j < N; j ++){
				{
					auto iter = tree.begin();
					int i = 0;
/*					if constexpr(T::with_value::value){
						print("i", i, data[i], iter->first);
					}else{
						print("i", i, data[i], *iter);
					}
*/					while(true){
						if(coin(engine) == 0){
							iter ++;
							i ++;
						}else{
							iter --;
							i --;
						}
						if(iter == tree.end()){
							break;
						}
						if constexpr(T::with_value::value){
//							print(i, data[i], iter->first);
							if(iter->first != data[i]){
								print(data);
								print(tree);
								cout << "iter error1" << endl;
								throw "ERROR";
							}
						}else{
//							print(i, data[i], *iter);
							if(*iter != data[i]){
								print(data);
								print(tree);
								cout << "iter error2" << endl;
								throw "ERROR";
							}
						}
					}
				}
				{
					auto iter = tree.rbegin();
					int i = data.size() - 1;
					while(true){
						if(coin(engine) == 0){
							iter ++;
							i --;
						}else{
							iter --;
							i ++;
						}
						if(iter == tree.end()){
							break;
						}
						if constexpr(T::with_value::value){
							if(iter->first != data[i]){
								print(i, data[i], iter->first);
								print(data);
								print(tree);
								cout << "iter error3" << endl;
								throw "ERROR";
							}
						}else{
							if(*iter != data[i]){
								print(i, data[i], *iter);
								print(data);
								print(tree);
								cout << "iter error4" << endl;
								throw "ERROR";
							}
						}
					}
				}
			}
		}
	}
};


template<typename T> class is_iterable{
public:
	static auto test(const T& c){
		for(const auto& v :  c){
			return v;
		}
	}
};


int main(void){
	avltree::map<int, int, avltree::tree_spec::with_index> x;
	
	x.insert(1, 10);
	x.insert(2, 10);
	x.insert(3, 10);
	x.insert(4, 10);
//	x.print();
	
	{
		auto& [k, v] = *x.find(1);
		print(k, v);
		print(k, v);
		print(k, v);
	}
	{
		auto f = x.find(1);
		if(f){
			auto& [k, v] = *f;
		}
	}
	{
		auto f = x.find(1);
		if(f){
			auto [k, v] = *f;
		}
	}

	{
		auto [k, v] = *x.remove(1);
		print(k, v);
		print(k, v);
		print(k, v);
	}
	{
		auto& hoge = *x.remove(2);
		auto& [k, v] = hoge;
		print(k, v);
		auto f = x.find(3);
		print(k, v);
		auto g = x.find(4);
		print(k, v);
		print(hoge);
	}
	{
		auto f = x.remove(3);
		if(f){
			auto& [k, v] = *f;
			print(k, v);
		}
	}
	{
		auto f = x.remove(4);
		if(f){
			auto [k, v] = *f;
			print(k, v);
		}
	}
	
/*
	if(!checker::check_tree(x)){
		throw "hoge";
	}
*/	
/*
	for(int i = 0; i < 100; i ++){
		x.insert(i, 10);
	
		x.print();
		if(!checker::check_tree(x)){
			throw "hoge";
		}
	}

	for(int i = 0; i < 100; i ++){
		x.remove(i);
	
		x.print();
		if(!checker::check_tree(x)){
			throw "hoge";
		}
	}
*/	
//	return 0;
/*	
	avltree::multiset<int, avltree::tree_spec::with_index> ms;
	ms.insert(1);
	ms.insert(0);
	ms.insert(2);
	ms.insert(0);
	ms.insert(3);
	ms.insert(1);
	
	ms.print();
	
	print(ms.index(1));
	print(ms.last_index(1));
	auto [v1, i1] = ms.find_lt_with_index(1);
	print(*v1, i1);
	auto [v2, i2] = ms.find_gt_with_index(1);
	print(*v2, i2);
*/	
	const int N = 1000;
	
#define CHECK_MAP(K, V, S, N)   { checker::test<avltree::map<K, V, S>,   std::map<K, V>>(N);   std::cout << "map<" << #K << ", " << #V << ", " << #S << ">" << std::endl; }
#define CHECK_SET(V, S, N)      { checker::test<avltree::set<V, S>,      std::set<V>>(N);      std::cout << "set<" << #V << ", " << #S << ">" << std::endl; }
#define CHECK_MULTISET(V, S, N) { checker::test<avltree::multiset<V, S>, std::multiset<V>>(N); std::cout << "multiset<" << #V << ", " << #S << ">" << std::endl; }

	CHECK_MAP(int64_t, int64_t, avltree::tree_spec::simple, N);
	CHECK_MAP(int64_t, int64_t, avltree::tree_spec::with_index, N);
	CHECK_MAP(int64_t, int64_t, avltree::tree_spec::with_depth, N);
	CHECK_MAP(int64_t, int64_t, avltree::tree_spec::with_index | avltree::tree_spec::with_depth, N);

	CHECK_SET(int64_t, avltree::tree_spec::simple, N);
	CHECK_SET(int64_t, avltree::tree_spec::with_index, N);
	CHECK_SET(int64_t, avltree::tree_spec::with_depth, N);
	CHECK_SET(int64_t, avltree::tree_spec::with_index | avltree::tree_spec::with_depth, N);

	CHECK_MULTISET(int64_t, avltree::tree_spec::simple, N);
	CHECK_MULTISET(int64_t, avltree::tree_spec::with_index, N);
	CHECK_MULTISET(int64_t, avltree::tree_spec::with_depth, N);
	CHECK_MULTISET(int64_t, avltree::tree_spec::with_index | avltree::tree_spec::with_depth, N);

	return 0;
}

