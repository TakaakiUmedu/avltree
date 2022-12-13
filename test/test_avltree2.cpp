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

int main(void){
	const int N = 500;
	const int64_t M = 100;
	std::random_device seed_gen;
	std::mt19937 engine(0);
	std::uniform_int_distribution<> dist(0, M);
	{
		avltree::set<int64_t, avltree::tree_spec::with_index>::with_summary_sum::with_summary_prod tree;
		vector<int64_t> vec;
		
		for(int i = 0; i < N; i ++){
			const int64_t x = dist(engine);
			const auto y = find(vec.begin(), vec.end(), x);
			if(y == vec.end()){
				vec.push_back(x);
				sort(vec.begin(), vec.end());
				tree.insert(x);
			}else{
				vec.erase(y);
				tree.remove(x);
			}
			for(int j = 0; j < vec.size(); j ++){
				for(int k = j + 1; k <= vec.size(); k ++){
					int64_t s1 = 0;
					int64_t p1 = 1;
					for(int l = j; l < k; l ++){
						s1 += vec[l];
						p1 *= vec[l];
					}
					const auto [s2, p2] = tree.summarize_by_index(j, k);
					if(s1 != s2 || p1 != p2){
						cout << "ERROR" << endl;
						print(vec, j, k, s1, p1);
						print(tree, j, k, s2, p2);
						return 1;
					}
				}
			}
		}
		cout << "set passed with size: " << vec.size() << endl;
	}

	{
		avltree::multiset<int64_t, avltree::tree_spec::with_index>::with_summary_sum::with_summary_prod tree;
		vector<int64_t> vec;
		
		for(int i = 0; i < N; i ++){
			if(vec.size() > 1 && dist(engine) < 20){
				std::uniform_int_distribution<> d(0, vec.size() - 1);
				const size_t i = d(engine);
				tree.remove(vec[i]);
				vec.erase(vec.begin() + i);
			}else{
				const int64_t x = dist(engine);
				vec.push_back(x);
				sort(vec.begin(), vec.end());
				tree.insert(x);
			}
			for(int j = 0; j < vec.size(); j ++){
				for(int k = j + 1; k <= vec.size(); k ++){
					int64_t s1 = 0;
					int64_t p1 = 1;
					for(int l = j; l < k; l ++){
						s1 += vec[l];
						p1 *= vec[l];
					}
					const auto [s2, p2] = tree.summarize_by_index(j, k);
					if(s1 != s2 || p1 != p2){
						cout << "ERROR" << endl;
						print(vec, j, k, s1, p1);
						print(tree, j, k, s2, p2);
						return 1;
					}
				}
			}
		}
		cout << "multiset passed with size: " << vec.size() << endl;
	}

	{
		avltree::map<int64_t, int64_t, avltree::tree_spec::with_index>::with_summary_key_sum::with_summary_key_prod::with_summary_value_sum::with_summary_value_prod::with_summary_value_min::with_summary_value_max tree;
		map<int64_t, int64_t> values;
		
		for(int i = 0; i < N; i ++){
			const int64_t x = dist(engine);
			if(values.size() > 1 && values.find(x) != values.end() && dist(engine) < 20){
				values.erase(x);
				tree.remove(x);
			}else{
				const int64_t y = dist(engine);
				values[x] = y;
				tree.insert(x, y);
			}
			vector<int64_t> vec1;
			vector<int64_t> vec2;
			for(const auto& [k, v]: values){
				vec1.push_back(k);
				vec2.push_back(v);
			}
			for(int j = 0; j < vec1.size(); j ++){
				for(int k = j + 1; k <= vec1.size(); k ++){
					int64_t s1 = 0;
					int64_t p1 = 1;
					int64_t s2 = 0;
					int64_t p2 = 1;
					int64_t min1 = *min_element(vec2.begin() + j, vec2.begin() + k);
					int64_t max1 = *max_element(vec2.begin() + j, vec2.begin() + k);
					for(int l = j; l < k; l ++){
						s1 += vec1[l];
						p1 *= vec1[l];
						s2 += vec2[l];
						p2 *= vec2[l];
					}
					const auto [s3, p3, s4, p4, min2, max2] = tree.summarize_by_index(j, k);
					if(s1 != s3 || p1 != p3 || s2 != s4 || p2 != p4 || min1 != min2 || max1 != max2){
						cout << "ERROR" << endl;
						print(values, j, k, s1, p1, s2, p2);
						print(tree, j, k, s3, p3, s4, p4);
						return 1;
					}else{
//						print(tree, j, k, s3, p3, s4, p4);
					}
				}
			}
		}
		cout << "map passed with size: " << values.size() << endl;
	}
	
	
	
	return 0;
}

