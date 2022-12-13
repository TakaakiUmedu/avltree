#include <memory>
#include <iostream>
#include <vector>
#include <cassert>
#include <functional>
#include <tuple>

namespace avltree{
	enum class tree_spec: int{
		simple            = 0,
		with_index        = 1 << 1,
		with_depth        = 1 << 2,
		pass_key_by_ref   = 1 << 3,
		pass_key_by_val   = 1 << 4,
		pass_value_by_ref = 1 << 5,
		pass_value_by_val = 1 << 6,
		max               = 1 << 7,
	};
	
	inline constexpr tree_spec operator|(const tree_spec v1, const tree_spec v2){ return static_cast<tree_spec>(static_cast<int>(v1) | static_cast<int>(v2)); }
	inline constexpr bool tree_spec_has(const tree_spec v1, const tree_spec v2){ return (static_cast<int>(v1) & static_cast<int>(v2)) != 0; }
	
	namespace avltree_base{
		class empty{
			empty() = delete;
		};
		template<typename K, typename V, tree_spec S, typename U = std::tuple<>> class avltree;
	}
	template<typename K, typename V, tree_spec S, typename U> class map;
	template<typename V, tree_spec S, typename U> class multiset;
	
	namespace summary{
		template<typename T> inline T pass(const T& d){ return d; }
		template<typename D, typename T> inline T get_key(const D& d){ return d.first; }
		template<typename D, typename T> inline T get_value(const D& d){ return d.second; }
		
		template<typename T> inline T add(const T& a, const T& b){ return a + b; }
		template<typename T> inline T mul(const T& a, const T& b){ return a * b; }
		template<typename T> inline T max(const T& a, const T& b){ return std::max(a, b); }
		template<typename T> inline T min(const T& a, const T& b){ return std::min(a, b); }
		
		template<typename T> inline T one(){ return 1; }
		template<typename T> inline T zero(){ return 0; }
		template<typename T> inline T min_value(){ return std::numeric_limits<T>::min(); }
		template<typename T> inline T max_value(){ return std::numeric_limits<T>::max(); }

		template<typename U, typename T> using tuple_append = decltype(std::tuple_cat(std::declval<U>(), std::make_tuple(std::declval<T>())));
		template<typename U, size_t I> struct summarizer_tuple_converter{
			using tuple_type = tuple_append<typename summarizer_tuple_converter<U, I - 1>::tuple_type, typename std::tuple_element_t<I - 1, U>::type>;
		};
		template<typename U> struct summarizer_tuple_converter<U, 0>{
			using tuple_type = std::tuple<>;
		};
		
		template<typename K, typename V, tree_spec S, typename T, T summarize(const T& a, const T& b), T identity(), T get(const typename avltree_base::avltree<K, V, S>::data_type&) = pass<T>> struct summarizer{
			using data_type = typename avltree_base::avltree<K, V, S>::data_type;
			using type = T;
			inline static T get_(const data_type& d)          { return get(d); }
			inline static T summarize_(const T& a, const T& b){ return summarize(a, b); }
			inline static T identity_()                       { return identity(); }
			inline static void accumulate_(T& a, const T& b)  { a = summarize(a, b); }
		};
		
		template<typename K, typename V, tree_spec S, typename U> struct summarizer_combined{
			using data_type = typename avltree_base::avltree<K, V, S>::data_type;
			using type = typename summarizer_tuple_converter<U, std::tuple_size_v<U>>::tuple_type;
		private:
			template<size_t I> static inline auto get__(const data_type& d){
				const auto t = std::make_tuple(std::tuple_element_t<I, U>::get_(d));
				if constexpr(I + 1 < std::tuple_size_v<U>){
					return std::tuple_cat(t, get__<I + 1>(d));
				}else{
					return t;
				}
			}
			template<size_t I> static inline auto summarize__(const type& a, const type& b){
				const auto t = std::make_tuple(std::tuple_element_t<I, U>::summarize_(std::get<I>(a), std::get<I>(b)));
				if constexpr(I + 1 < std::tuple_size_v<U>){
					return std::tuple_cat(t, summarize__<I + 1>(a, b));
				}else{
					return t;
				}
			}
			template<size_t I> static inline auto identity__(){
				const auto t = std::make_tuple(std::tuple_element_t<I, U>::identity_());
				if constexpr(I + 1 < std::tuple_size_v<U>){
					return std::tuple_cat(t, identity__<I + 1>());
				}else{
					return t;
				}
			}
		public:
			static inline type get_(const data_type& d){ return get__<0>(d); }
			static inline type summerize_(const type& a, const type& b){ return summarize__<0>(a, b); }
			static inline type identity_(){ return identity__<0>(); }
			static inline void accumulate_(type& a, const type& b){ a = summarize__<0>(a, b); }
		};

		template<typename K, typename V, tree_spec S, typename U> using tuple_to_summarizer = 
			std::conditional_t<std::tuple_size_v<U> == 0, avltree_base::empty, 
			std::conditional_t<std::tuple_size_v<U> == 1, std::tuple_element_t<0, tuple_append<U, int>>, // to avoid error caused by std::tuple_element<0, tuple<>>
					summarizer_combined<K, V, S, U>>>;
	}
//	template<typename K, typename V, tree_spec S> class summarizer;
	
	namespace avltree_base{
		template<typename I, typename O, typename T, T F(O)> class iterator_wrapper{
			I i;
		public:
			inline iterator_wrapper(I&& i_): i(std::move(i_)){}
			template<typename E> inline bool operator==(const E& e) const { return i == e; };
			template<typename E> inline bool operator!=(const E& e) const { return i != e; };
			template<typename E> friend inline bool operator==(const E& e, const iterator_wrapper& i) { return e == i.i; };
			template<typename E> friend inline bool operator!=(const E& e, const iterator_wrapper& i) { return e != i.i; };
			inline T operator*() const { return F(*i); };
			inline iterator_wrapper& operator++(){ ++ i; return *this; }
			inline iterator_wrapper& operator--(){ -- i; return *this; }
			inline iterator_wrapper operator++(int){ auto _i = *this; i.i ++; return _i; }
			inline iterator_wrapper operator--(int){ auto _i = *this; i.i --; return _i; }
		};
		
		template<typename B, typename E> class iterator_pair{
			const B b;
			const E e;
		public:
			inline iterator_pair(B&& b_, E&& e_): b(std::move(b_)), e(std::move(e_)){}
			inline auto begin(){ return b(); }
			inline auto end(){ return e(); }
		};
		
		template <size_t N = 0, typename... T> static std::string to_string(const std::tuple<T...>& v){
			std::string result = "";
			if constexpr(N == 0){
				result += "(";
			}
			result += to_string(std::get<N>(v));
			if constexpr(N == sizeof...(T) - 1){
				result += ")";
			}else{
				result += "," + to_string<N + 1, T...>(v);
			}
			return result;
		}
		
		template <typename T> static std::string to_string(const T& v){
			return std::to_string(v);
		}
		

		template<typename K, typename V, tree_spec S, typename U> class avltree{
			friend class ::avltree::multiset<K, S, U>;
#ifdef AVLTREE_DEBUG_CLASS
				friend class AVLTREE_DEBUG_CLASS;
#endif
			static_assert(!tree_spec_has(S, tree_spec::max), "invalid tree_spec");
			static_assert(!(tree_spec_has(S, tree_spec::pass_key_by_ref) && tree_spec_has(S, tree_spec::pass_key_by_val)), "invalid tree_spec: both flags passing key by reference and value cannot be specified at same time");
			static_assert(!(tree_spec_has(S, tree_spec::pass_value_by_ref) && tree_spec_has(S, tree_spec::pass_value_by_val)), "invalid tree_spec: both flags passing value by reference and value cannot be specified at same time");
			avltree() = delete;
		public:
			using with_depth    = std::conditional_t<tree_spec_has(S, tree_spec::with_depth), std::true_type, std::false_type>;
			using with_index    = std::conditional_t<tree_spec_has(S, tree_spec::with_index), std::true_type, std::false_type>;
			using with_summary  = std::negation<std::is_same<U, std::tuple<>>>;
			using with_value    = std::negation<std::is_same<V, empty>>;
			
			using use_ref_k = std::bool_constant<(tree_spec_has(S, tree_spec::pass_key_by_ref) || (!tree_spec_has(S, tree_spec::pass_key_by_val) && sizeof(std::tuple<K>) > sizeof(std::nullptr_t)))>;
			using use_ref_v = std::bool_constant<(tree_spec_has(S, tree_spec::pass_value_by_ref) || (!tree_spec_has(S, tree_spec::pass_value_by_val) && sizeof(std::tuple<V>) > sizeof(std::nullptr_t)))>;
			
			using KR = std::conditional_t<use_ref_k::value, const K&, const K>;
			using VR = std::conditional_t<use_ref_v::value, const V&, const V>;
			
			using data_type = std::conditional_t<with_value::value, std::pair<K, V>, K>;

		private:
			class node_with_value;
			class node_without_value;
			using node = std::conditional_t<with_value::value, node_with_value, node_without_value>;
//			class node;
			using node_uptr = std::unique_ptr<node>;
		private:
			static std::string node_to_string(const node& node){
				std::string ret = "(" + avltree_base::to_string(node.key());
				
				if constexpr(with_index::value){
					ret += ", c=" + avltree_base::to_string(node.c);
				}
				if constexpr(with_depth::value){
					ret += ", h=" + avltree_base::to_string(node.h);
				}else{
					ret += ", b=" + avltree_base::to_string(node.balance());
				}
				
				ret += ")";
				if constexpr(with_value::value){
					ret += "-> " + avltree_base::to_string(node.value());
				}
				
				return ret;
			}
			static void print_node(const node& node, bool top, std::string indent, bool lr_flag){
				if(node.l){
					print_node(*node.l, false, top ? "" : indent + (lr_flag ? "   " : "  |"), true);
				}
				
				if(top){
					std::cout << "-" << node_to_string(node) << std::endl;
					if(node.r){
						std::cout << "  |" << std::endl;
					}
				}else{
					std::cout << indent << "  +" << node_to_string(node) << std::endl;
					
					if(lr_flag || node.r || indent.find("|") != std::string::npos){
						std::cout << indent;
						if(lr_flag && node.r){
							std::cout << "  |  |" << std::endl;
						}else if(lr_flag){
							std::cout << "  |" << std::endl;
						}else if(node.r){
							std::cout << "     |" << std::endl;
						}else{
							std::cout << std::endl;
						}
					}
				}
				if(node.r){
					print_node(*node.r, false, top ? "" : indent + (lr_flag ? "  |" : "   "), false);
				}
			}
			
			/*
				NODE BASE : node_base0
			*/
			
			struct node_base0{
				std::unique_ptr<node> l, r;
			protected:
				inline node_base0() : l(), r() {}
			};
			
			/*
				WITH BALANCE or DEPTH : node_base1
			*/
			
			struct node_with_balance: public node_base0{
				int b;
				inline int balance() const { return b; }
			protected:
				inline node_with_balance(): node_base0(), b(0) {}
			};
			
			struct node_with_depth: public node_base0{
				int h;
				inline int depth_l() const { return this->l ? this->l->h : 0; }
				inline int depth_r() const { return this->r ? this->r->h : 0; }
				inline void reset_depth(){
					h = std::max(depth_l(), depth_r()) + 1;
				}
				inline int balance() const { return depth_l() - depth_r(); }
			protected:
				inline node_with_depth(): node_base0(), h(1) {}
			};
			
			using node_base1 = std::conditional_t<with_depth::value, node_with_depth, node_with_balance>;
			
			/*
				WITH COUNT(INDEX) or not : node_base2
			*/
			
			struct node_with_count : public node_base1{
				size_t c;
			protected:
				inline node_with_count() : node_base1(), c(1) {}
			public:
				inline void update_count(){
					const auto cl = this->l ? this->l->c : 0;
					const auto cr = this->r ? this->r->c : 0;
					c = cl + cr + 1;
				}
			};
			
			using node_base2 = std::conditional_t<with_index::value, node_with_count, node_base1>;
			
			/*
				WITH VALUE or not : node_base3
			*/
			
			struct node_base_with_value : public node_base2{
				data_type data;
				inline KR key() const { return data.first; }
				inline K& key(){ return data.first; }
				inline VR value() const{ return data.second; }
				inline V& value(){ return data.second; }
				inline node_base_with_value(data_type&& data_): data(std::move(data_)){}
			};
			struct node_base_without_value : public node_base2{
				data_type data;
				inline KR key() const { return data; }
				inline K& key(){ return data; }
				inline node_base_without_value(data_type&& data_): data(std::move(data_)){}
			};
			
			using node_base3 = std::conditional_t<with_value::value, node_base_with_value, node_base_without_value>;
			
			/*
				WITH SUMMARY or not : node_base4
			*/
			
			using summarizer = summary::tuple_to_summarizer<K, V, S, U>;
			
			struct node_with_summary : public node_base3{
				typename summarizer::type s;
			protected:
				node_with_summary(data_type&& data_): node_base3(std::move(data_)), s(summarizer::identity_())/*, k_min(nullptr), k_max(nullptr)*/{ update_summary(); }
			public:
				inline void update_summary(){
					s = summarizer::get_(this->data);
					if(this->l){
						summarizer::accumulate_(s, this->l->s);
					}
					if(this->r){
						summarizer::accumulate_(s, this->r->s);
					}
				}
			};
			
			using node_base4 = std::conditional_t<with_summary::value, node_with_summary, node_base3>;
			
			/*
				WITH VALUE or not
			*/
			
			struct node_with_value: node_base4{
				inline node_with_value(const K& k, const V&  v): node_base4(data_type(k, v)){}
				inline node_with_value(const K& k, V&& v):       node_base4(data_type(k, std::move(v))){}
				inline node_with_value(K&& k, const V&  v):      node_base4(data_type(std::move(k), v)){}
				inline node_with_value(K&& k, V&& v):            node_base4(data_type(std::move(k), std::move(v))){}
				inline void print() const{ print_node(*this, true, "", true); }
			};
			
			struct node_without_value: node_base4{
				inline node_without_value(const K& k): node_base4(data_type(k)){}
				inline node_without_value(K&& k):      node_base4(data_type(std::move(k))){}
				inline void print() const{ print_node(*this, true, "", true); }
			};
			
			template<typename N> struct node_view_base0{
			protected:
				N n;
				inline node_view_base0(const N& _n): n(_n){}
				inline node_view_base0(N&& _n): n(std::move(_n)){}
			public:
				inline operator bool() const { return n != nullptr; }
				inline bool operator==(std::nullptr_t n) const { return !*this; }
				inline const data_type& operator*() const& { return n->data; }
				inline const data_type operator*() && { return std::move(n->data); }
				inline data_type const* operator->() const& { return &n->data; }
			};
			
			template<typename N> struct node_view_with_count: public node_view_base0<N>{
			protected:
				inline node_view_with_count(const N& n_): node_view_base0<N>(n_){}
				inline node_view_with_count(N&& n_): node_view_base0<N>(std::move(n_)){}
			public:
				inline size_t size() const { return this->n->count; }
			};
			
			template<typename N> using node_view_base1 = std::conditional_t<with_index::value, node_view_with_count<N>, node_view_base0<N>>;
			
			template<typename N> struct node_view_with_depth: public node_view_base1<N>{
			protected:
				inline node_view_with_depth(const N& n_): node_view_base1<N>(n_){}
				inline node_view_with_depth(N&& n_): node_view_base1<N>(std::move(n_)){}
			public:
				inline size_t height() const { return this->n->h; }
			};
			
			template<typename N> using node_view_base2 = std::conditional_t<with_depth::value, node_view_with_depth<N>, node_view_base1<N>>;
			
		public:
			class node_view: public node_view_base2<const node*>{
			public:
				inline explicit node_view(const node* n_): node_view_base2<const node*>(n_){}
				inline explicit node_view(const node_uptr& n_): node_view_base2<const node*>(n_.get()){};
				inline friend bool operator==(std::nullptr_t p, const node_view& n) { return !n; }
			};
			
			class node_uptr_view: public node_view_base2<node_uptr>{
			public:
				inline explicit node_uptr_view(node_uptr&& n_): node_view_base2<node_uptr>(std::move(n_)){}
				inline friend bool operator==(std::nullptr_t p, const node_uptr_view& n) { return !n; }
			};
			
		private:
			using branch_func = std::function<int(const node&)>;
			
			template<bool L2R = true> class iterator_base: public node_view{
				std::vector<const node*> _stack;
				int _depth;
				const node* cur() const { return this->n; }
				const node* &cur(){ return this->n; }
				template<bool F> inline void _down_to_leaf(){
					while(true){
						const auto& c = (L2R ^ !F) ? cur()->l : cur()->r;
						if(!c){
							break;
						}
						_stack[_depth] = cur();
						_depth ++;
						cur() = c.get();
					}
				}
				template<bool F> inline void _forward(){
					const auto& c = (L2R ^ !F) ? cur()->r : cur()->l;
					if(c){
						_stack[_depth] = cur();
						_depth ++;
						cur() = c.get();
						_down_to_leaf<F>();
					}else{
						while(true){
							if(_depth > 0){
								auto child = cur();
								_depth --;
								cur() = _stack[_depth];
								if(((L2R ^ !F) ? cur()->l.get() : cur()->r.get()) == child){
									break;
								}
							}else{
								cur() = nullptr;
								break;
							}
						}
					}
				}
			public:
				inline iterator_base(size_t stack_size, const node* root) : _stack(stack_size), _depth(0), node_view(root){
					if(cur()){
						_down_to_leaf<true>();
					}
				}
				inline iterator_base(size_t stack_size, const node* root, branch_func branch) : _stack(stack_size), _depth(0), node_view(root){
					while(cur()){
						const int b = branch(*cur());
						if(b == 0){
							break;
						}else if(b == -1){
							cur() = cur()->l.get();
						}else{
							cur() = cur()->r.get();
						}
					}
				}
				inline bool operator==(std::nullptr_t _) const{ return cur() == nullptr; }
				inline bool operator!=(std::nullptr_t _) const{ return cur() != nullptr; }
				inline iterator_base& operator++(){ _forward<true>(); return *this;}
				inline iterator_base& operator--(){ _forward<false>(); return *this; }
				inline iterator_base operator++(int){ auto i = *this; ++*this; return i; }
				inline iterator_base operator--(int){ auto i = *this; --*this; return i; }
			};
			
			class tree_base0{
			protected:
				tree_base0() : root(), stack(){
					this->stack.push_back(nullptr);
				}
				inline static int _branch_eq(const node& cur, KR k){ return cur.key() == k ? 0 : (k < cur.key() ? -1 : 1); }
				inline static branch_func _make_branch_eq(KR k){
					if constexpr(use_ref_k::value){
						return [&k](const node& cur){ return _branch_eq(cur, k); };
					}else{
						return [k](const node& cur){ return _branch_eq(cur, k); };
					}
				}
				inline static int _branch_ge(const node& cur, const node*& cand, KR k){
					if(k <= cur.key() && (!cand || cur.key() < cand->key())){
						cand = &cur;
					}
					return _branch_eq(cur, k);
				}
				inline static int _branch_le(const node& cur, const node*& cand, KR k){
					if(cur.key() <= k && (!cand || cand->key() < cur.key())){
						cand = &cur;
					}
					return _branch_eq(cur, k);
				}
				inline static int _branch_gt(const node& cur, const node*& cand, KR k){
					if(k < cur.key()){
						if(!cand || cur.key() < cand->key()){
							cand = &cur;
						}
						return -1;
					}else{
						return 1;
					}
				}
				inline static int _branch_lt(const node& cur, const node*& cand, KR k){
					if(cur.key() < k){
						if(!cand || cand->key() < cur.key()){
							cand = &cur;
						}
						return 1;
					}else{
						return -1;
					}
				}
				template<int B(const node&, const node*&, KR)> inline static branch_func _make_branch_find_func(const node*& cand, KR k){
					if constexpr(use_ref_k::value){
						return [&cand, &k](const node& cur){ return B(cur, cand, k); };
					}else{
						return [&cand, k](const node& cur){ return B(cur, cand, k); };
					}
				}
				inline static branch_func _make_branch_l(){ return [](const node& cur){ return cur.l ? -1 : (cur.r ? 1 : 0); }; }
				inline static branch_func _make_branch_r(){ return [](const node& cur){ return cur.r ? 1 : (cur.l ? -1 : 0); }; }
				inline static branch_func _make_branch_at(size_t& index){
					return [&index](const node& cur){
						const auto count_l = cur.l ? cur.l->c : 0;
						if(index == count_l){
							return 0;
						}else if(index < count_l){
							return -1;
						}else{
							index -= count_l + 1;
							return 1;
						}
					};
				}
				template <int B(const node&, KR)> inline static branch_func _make_branch_calc_index(size_t& index, KR k){
					return [&index, k](const node& cur){
						const auto b = B(cur, k);
						if(b >= 0){
							if(cur.l){
								index += cur.l->c;
							}
							if(b == 1){
								index += 1;
							}
						}
						return b;
					};
				}
				
				node_uptr root;
				std::vector<node_uptr*> stack;
			
				inline std::tuple<node_uptr&, size_t> _find(branch_func branch){
					node_uptr* cur = &this->root;
					size_t height = 0;
					while(*cur){
						const auto b = branch(**cur);
						if(b == 0){
							break;
						}
						this->stack[height] = cur;
						height ++;
						if(b < 0){
							cur = &(*cur)->l;
						}else{
							cur = &(*cur)->r;
						}
					}
					return std::tuple_cat(std::tie(*cur), std::make_tuple(height));
				}
			
				inline const node* _find(branch_func branch) const{
					const node* cur = this->root.get();
					while(cur){
						const auto b = branch(*cur);
						if(b == 0){
							break;
						}else if(b < 0){
							cur = cur->l.get();
						}else{
							cur = cur->r.get();
						}
					}
					return cur;
				}
				
				template<int B(const node&, const node*&, KR)> inline const node* _find_nearest(KR k) const{
					const node* cand = nullptr;
					this->_find(tree_base0::template _make_branch_find_func<B>(cand, k));
					return cand;
				}
				
#ifdef AVLTREE_DEBUG_CLASS
				friend class AVLTREE_DEBUG_CLASS;
#endif
			};
			
			class tree_base_without_index : public tree_base0{
			protected:
				size_t _count;
				tree_base_without_index() : tree_base0(), _count(0){}
			public:
				size_t size() const{ return _count; }
			};
			
		public:
			using iterator         = avltree<K, V, S, U>::iterator_base<true>;
			using reverse_iterator = avltree<K, V, S, U>::iterator_base<false>;
		private:
			class tree_base_with_index : public tree_base0{
			protected:
				tree_base_with_index() : tree_base0(){}
			public:
				inline size_t size() const{ return this->root ? this->root->c : 0; }
				
				inline const node_view at(size_t index) const{ return node_view(this->_find(tree_base0::_make_branch_at(index))); }
				inline node_uptr_view pop_at(size_t index){ return node_uptr_view(_pop(tree_base0::_make_branch_at(index))); }
				inline iterator iterator_at(size_t index) const{ return iterator(this->stack.size(), this->root.get(), tree_base0::_make_branch_at(index)); }
				
				inline size_t index(KR k) const{
					size_t index = 0;
					if(this->_find(tree_base0::template _make_branch_calc_index<tree_base0::_branch_eq>(index, k))){
						return index;
					}else{
						return size();
					}
				}
				
				template<int B(const node&, const node*&, KR)> inline std::tuple<const node_view, size_t> _find_nearest_with_index(KR k) const {
					const node* cand = nullptr;
					size_t index = this->size(), index_tmp = 0;
					const auto branch_base = tree_base0::template _make_branch_find_func<B>(cand, k);
					const auto branch = [&index, &index_tmp, &cand, branch_base](const node& cur){
						const int b = branch_base(cur);
						if(&cur == cand){
							index = index_tmp + (cur.l ? cur.l->c : 0);
						}
						if(b >= 1){
							index_tmp += cur.l ? cur.l->c : 0;
							if(b == 1){
								index_tmp += 1;
							}
						}
						return b;
					};
					this->_find(branch);
					return std::make_tuple(node_view(cand), index);
				}
				
				inline std::tuple<const node_view, size_t> find_ge_with_index(KR k) const{ return _find_nearest_with_index<tree_base0::_branch_ge>(k); }
				inline std::tuple<const node_view, size_t> find_gt_with_index(KR k) const{ return _find_nearest_with_index<tree_base0::_branch_gt>(k); }
				inline std::tuple<const node_view, size_t> find_le_with_index(KR k) const{ return _find_nearest_with_index<tree_base0::_branch_le>(k); }
				inline std::tuple<const node_view, size_t> find_lt_with_index(KR k) const{ return _find_nearest_with_index<tree_base0::_branch_lt>(k); }
			};
			
			using tree_base1 = std::conditional_t<with_index::value, tree_base_with_index, tree_base_without_index>;

		public:
			
			class tree_base2 : public tree_base1{
				template<node_uptr& L(node&), node_uptr& R(node&), int D> inline void _rotate(node_uptr& p1){
					node_uptr& p2 = L(*p1);
					node_uptr& p3 = R(*p2);
					if(p2->balance() == -D){
						node_uptr& p4 = L(*p3);
						node_uptr& p5 = R(*p3);
						const auto p3b = p3->balance();
						swap(p1, p3);
						swap(p2, p5);
						swap(p3, p4);
						swap(p4, p5);
						if constexpr(with_depth::value){
							p4->reset_depth();
							p5->reset_depth();
							p1->reset_depth();
						}else{
							p1->b = 0;
							if(p3b == 0){
								p4->b = 0;
								p5->b = 0;
							}else if(p3b == D){
								p4->b = 0;
								p5->b = -D;
							}else{
								p4->b = D;
								p5->b = 0;
							}
						}
						if constexpr(with_index::value){
							p4->update_count();
							p5->update_count();
							p1->update_count();
						}
						if constexpr(with_summary::value){
							p4->update_summary();
							p5->update_summary();
							p1->update_summary();
						}
					}else{
						swap(p1, p2);
						swap(p2, p3);
						if constexpr(with_depth::value){
							p3->reset_depth();
							p1->reset_depth();
						}else{
							p1->b -= D;
							if(p1->b == 0){
								p3->b = 0;
							}else{
								p3->b = D;
							}
						}
						if constexpr(with_index::value){
							p3->update_count();
							p1->update_count();
						}
						if constexpr(with_summary::value){
							p3->update_summary();
							p1->update_summary();
						}
					}
				}
				
				inline static node_uptr& get_l(node& node){ return node.l; }
				inline static node_uptr& get_r(node& node){ return node.r; }
				inline void _rotate_lr(node_uptr& p1){ _rotate<get_l, get_r, 1>(p1); }
				inline void _rotate_rl(node_uptr& p1){ _rotate<get_r, get_l, -1>(p1); }
			protected:
				template <typename K_> inline bool _insert(K_&& k){
					auto [new_node, height] = this->_find(tree_base0::_make_branch_eq(k));
					if(new_node){
						return false;
					}
					new_node = std::make_unique<node>(std::forward<K_>(k));
					_fix_balance<1>(new_node, height);
					return true;
				}
				template <typename K_, typename V_> inline bool _insert(K_&& k, V_&& v){
					auto [new_node, height] = this->_find(tree_base0::_make_branch_eq(k));
					if(new_node){
						new_node->value() = std::forward<V_>(v);
						if constexpr(with_summary::value){
							new_node->update_summary();
							while(height != 0){
								height --;
								(*this->stack[height])->update_summary();
							}
						}
						return false;
					}
					new_node = std::make_unique<node>(std::forward<K_>(k), std::forward<V_>(v));
					_fix_balance<1>(new_node, height);
					return true;
				}
				node_uptr _pop(branch_func branch){
					auto [node, height] = this->_find(branch);
					node_uptr* cur = &node;
					node_uptr release = nullptr;
					if(*cur){
						node_uptr& n = *cur;
						if(n->l){
							if(n->r){
								auto get_f = n->balance() >= 0 ? get_l : get_r;
								auto get_b = n->balance() >= 0 ? get_r : get_l;
								this->stack[height] = cur;
								height ++;
								cur = &get_f(**cur);
								while(get_b(**cur)){
									this->stack[height] = cur;
									height ++;
									cur = &get_b(**cur);
								}
								n->key() = std::move((*cur)->key());
								if constexpr(with_value::value){
									n->value() = std::move((*cur)->value());
								}
								std::swap(release, get_f(**cur));
								std::swap(*cur, release);
							}else{
								std::swap(release, n->l);
								std::swap(n, release);
							}
						}else{
							std::swap(release, n->r);
							std::swap(n, release);
						}
						if(height > 0){
							_fix_balance<-1>(*cur, height);
						}else{
							if constexpr(!with_index::value){
								this->_count --;
							}
						}
					}
					return release;
				}
				
				template<int D> inline void _fix_balance(node_uptr& cur, size_t height){
					node_uptr* child = &cur;
					if constexpr(!with_index::value){
						this->_count += D;
					}
					const auto max_depth = height;
					while(height > 0){
						height --;
						node_uptr* parent_ptr = this->stack[height];
						node& parent = **parent_ptr;
						if constexpr(with_index::value){
							parent.c += D;
						}
						if constexpr(with_depth::value){
							parent.reset_depth();
						}else{
							if constexpr(with_summary::value){
								parent.update_summary();
							}
							if(&parent.l == child){
								parent.b += D;
							}else{
								parent.b -= D;
							}
						}
						if(parent.balance() == 2){
							_rotate_lr(*parent_ptr);
							if constexpr(D == 1){
								break;
							}else{
								if((*parent_ptr)->balance() != 0){
									break;
								}
							}
						}else if(parent.balance() == -2){
							_rotate_rl(*parent_ptr);
							if constexpr(D == 1){
								break;
							}else{
								if((*parent_ptr)->balance() != 0){
									break;
								}
							}
						}else if(parent.balance() == 0){
							if constexpr(D == 1){
								break;
							}
						}else{
							if constexpr(D != 1){
								break;
							}
						}
						child = parent_ptr;
					}
					if(height == 0){
						if constexpr(D == 1){
							if(this->stack.size() <= max_depth){
								this->stack.push_back(nullptr);
							}
						}
					}else{
						if constexpr(with_depth::value || with_index::value || with_summary::value){
							while(height > 0){
								height --;
								node_uptr* parent_ptr = this->stack[height];
								node& parent = **parent_ptr;
								if constexpr(with_index::value){
									parent.c += D;
								}
								if constexpr(with_depth::value){
									parent.reset_depth();
								}
								if constexpr(with_summary::value){
									parent.update_summary();
								}
								child = parent_ptr;
							}
						}
					}
				}
				
				tree_base2() : tree_base1(){};
			public:
				node_uptr_view remove(KR k){ return node_uptr_view(_pop(tree_base0::_make_branch_eq(k))); }
				node_uptr_view pop_first() { return node_uptr_view(_pop(tree_base0::_make_branch_l())); }
				node_uptr_view pop_last()  { return node_uptr_view(_pop(tree_base0::_make_branch_r())); }
				
				bool contains(KR k) const{ return this->_find(tree_base0::_make_branch_eq(k)) != nullptr; }
				
				void print(){
					if(this->root){
						this->root->print();
					}
				}
				
				inline const node_view find(KR k) const{ return node_view(this->_find(tree_base0::_make_branch_eq(k))); }
				inline const iterator iterator_find(KR k) const{ return iterator(this->stack.size(), this->root.get(), tree_base0::_make_branch_eq(k)); }
				
				inline const node_view find_ge(KR k) const{ return node_view(this->template _find_nearest<tree_base0::_branch_ge>(k)); }
				inline const node_view find_gt(KR k) const{ return node_view(this->template _find_nearest<tree_base0::_branch_gt>(k)); }
				inline const node_view find_le(KR k) const{ return node_view(this->template _find_nearest<tree_base0::_branch_le>(k)); }
				inline const node_view find_lt(KR k) const{ return node_view(this->template _find_nearest<tree_base0::_branch_lt>(k)); }
				
				inline iterator begin() const{ return iterator(this->stack.size(), this->root.get()); }
				inline reverse_iterator rbegin() const{ return reverse_iterator(this->stack.size(), this->root.get()); }
				inline std::nullptr_t end() const{ return nullptr; }
				inline std::nullptr_t rend() const{ return end(); }
				
#ifdef AVLTREE_DEBUG_CLASS
				friend class AVLTREE_DEBUG_CLASS;
#endif
			};
			
			class tree_base_with_summary: public tree_base2{
			private:
				typename summarizer::type summarize_(const node& cur, KR l, KR r){
					typename summarizer::type summary(summarizer::identity_());
					if(cur.l && l <= cur.key()){
						if(r >= cur.key()){
							summarizer::accumulate_(summary, summarize_l_(*cur.l, l));
						}else{
							summarizer::accumulate_(summary, summarize_(*cur.l, l, r));
						}
					}
					if(l <= cur.key() && r >= cur.key()){
						summarizer::accumulate_(summary, summarizer::get_(cur.data));
					}
					if(cur.r && r >= cur.key()){
						if(l <= cur.key()){
							summarizer::accumulate_(summary, summarize_r_(*cur.r, r));
						}else{
							summarizer::accumulate_(summary, summarize_(*cur.r, l, r));
						}
					}
					return summary;
				}
				typename summarizer::type summarize_l_(const node& cur, KR l){
					if(l > cur.key()){
						if(cur.r){
							return summarize_l_(*cur.r, l);
						}else{
							return summarizer::identity_();
						}
					}else{
						typename summarizer::type summary(summarizer::get_(cur.data));
						if(cur.l){
							summarizer::accumulate_(summary, summarize_l_(*cur.l, l));
						}
						if(cur.r){
							summarizer::accumulate_(summary, cur.r->s);
						}
						return summary;
					}
				}
				typename summarizer::type summarize_r_(const node& cur, KR r){
					if(r < cur.key()){
						if(cur.l){
							return summarize_r_(*cur.l, r);
						}else{
							return summarizer::identity_();
						}
					}else{
						typename summarizer::type summary(summarizer::get_(cur.data));
						if(cur.r){
							summarizer::accumulate_(summary, summarize_r_(*cur.r, r));
						}
						if(cur.l){
							summarizer::accumulate_(summary, cur.l->s);
						}
						return summary;
					}
				}
			public:
				tree_base_with_summary(): tree_base2(){}
				inline typename summarizer::type summarize(KR l, KR r){
					if(this->root){
						return summarize_(*this->root, l, r);
					}else{
						return summarizer::identity_();
					}
				}
				inline typename summarizer::type summarize(const node_view& l, const node_view& r){
					return summarize(*l, *r);
				}
			};
			
			class tree_base_with_summary_and_index: public tree_base_with_summary{
				typename summarizer::type summarize_by_index_(const node& cur, size_t l, size_t r){
					if(l >= r){
						return summarizer::identity_();
					}else if(l == 0 && r >= cur.c){
						return cur.s;
					}else{
						typename summarizer::type summary(summarizer::identity_());
						size_t cl;
						if(cur.l){
							cl = cur.l->c;
							if(l < cl){
								summarizer::accumulate_(summary, summarize_by_index_(*cur.l, l, r));
							}
						}else{
							cl = 0;
						}
						if(l <= cl && r >= cl + 1){
							summarizer::accumulate_(summary, summarizer::get_(cur.data));
						}
						if(cur.r){
							if(r > cl + 1){
								summarizer::accumulate_(summary, summarize_by_index_(*cur.r, l > cl + 1 ? l - cl - 1 : 0, r - cl - 1));
							}
						}
						return summary;
					}
				}
			public:
				tree_base_with_summary_and_index(): tree_base_with_summary(){}
				typename summarizer::type summarize_by_index(size_t l, size_t r){
					if(this->root){
						return summarize_by_index_(*this->root, l, r);
					}else{
						return summarizer::identity_();
					}
				}
			};
			
			using tree_base = std::conditional_t<with_summary::value, std::conditional_t<with_index::value, tree_base_with_summary_and_index, tree_base_with_summary>, tree_base2>;
			
			
			class multiset_base0: public tree_base{
				inline static int _branch_eq(const node& cur, KR k){
					if(cur.key() == k){
						return cur.balance() > 0 ? 1 : -1;
					}else{
						return k < cur.key() ? -1 : 1;
					}
				}
				inline static branch_func _make_branch_eq(KR k){
					if constexpr(use_ref_k::value){
						return [&k](const node& cur){ return _branch_eq(cur, k); };
					}else{
						return [k](const node& cur){ return _branch_eq(cur, k); };
					}
				}
			protected:
				multiset_base0(): tree_base(){}
			public:
				template<typename V_> bool insert(V_&& v){
					auto [new_node, height] = this->_find(_make_branch_eq(v));
					new_node = std::make_unique<node>(std::forward<V_>(v));
					this->template _fix_balance<1>(new_node, height);
					return true;
				}
			};
			
			class multiset_with_index: public multiset_base0{
				template<int D> inline static int _branch_index(size_t& index, size_t& index_tmp, const node& cur, KR k){
					if(cur.key() == k){
						index = index_tmp + (cur.l ? cur.l->c : 0);
						if(D == 1){
							index_tmp += (cur.l ? cur.l->c : 0) + 1;
							return 1;
						}else{
							return -1;
						}
					}else if(k < cur.key()){
						return -1;
					}else{
						index_tmp += (cur.l ? cur.l->c : 0) + 1;
						return 1;
					}
				}
				inline static int _branch_ge(const node& cur, const node*& cand, KR k){
					if(k <= cur.key() && (!cand || cur.key() <= cand->key())){
						cand = &cur;
					}
					if(cur.key() < k){
						return 1;
					}else{
						return -1;
					}
				}
				inline static int _branch_le(const node& cur, const node*& cand, KR k){
					if(cur.key() <= k && (!cand || cand->key() <= cur.key())){
						cand = &cur;
					}
					if(k < cur.key()){
						return -1;
					}else{
						return 1;
					}
				}
				inline static int _branch_gt(const node& cur, const node*& cand, KR k){
					if(k < cur.key()){
						if(!cand || cur.key() <= cand->key()){
							cand = &cur;
						}
						return -1;
					}else{
						return 1;
					}
				}
				inline static int _branch_lt(const node& cur, const node*& cand, KR k){
					if(cur.key() < k){
						if(!cand || cand->key() <= cur.key()){
							cand = &cur;
						}
						return 1;
					}else{
						return -1;
					}
				}
				template<int D> inline size_t _index(KR k) const{
					size_t index = this->size(), index_tmp = 0;
					if constexpr(use_ref_k::value){
						this->_find([&k, &index, &index_tmp](const node&cur){ return _branch_index<D>(index, index_tmp, cur, k); });
					}else{
						this->_find([k, &index, &index_tmp](const node&cur){ return _branch_index<D>(index, index_tmp, cur, k); });
					}
					return index;
				}
			protected:
				multiset_with_index(): multiset_base0(){}
			public:
				inline size_t index(KR k) const{ return this->_index<-1>(k); }
				inline size_t last_index(KR k) const{ return this->_index<1>(k); }
				inline size_t count(KR k) const{
					size_t first = index(k);
					if(first == this->size()){
						return 0;
					}else{
						return last_index(k) - first + 1;
					}
				}
				inline std::tuple<const node_view, size_t> find_ge_with_index(KR k) const{ return this->template _find_nearest_with_index<_branch_ge>(k); }
				inline std::tuple<const node_view, size_t> find_le_with_index(KR k) const{ return this->template _find_nearest_with_index<_branch_le>(k); }
				inline std::tuple<const node_view, size_t> find_gt_with_index(KR k) const{ return this->template _find_nearest_with_index<_branch_gt>(k); }
				inline std::tuple<const node_view, size_t> find_lt_with_index(KR k) const{ return this->template _find_nearest_with_index<_branch_lt>(k); }
			};
			
			using multiset_base = std::conditional_t<with_index::value, multiset_with_index, multiset_base0>;
		};
	}
	
	template<typename K, typename V, tree_spec S = tree_spec::simple, typename U = std::tuple<>> class map: public avltree_base::avltree<K, V, S, U>::tree_base{
#ifdef AVLTREE_DEBUG_CLASS
		friend class AVLTREE_DEBUG_CLASS;
		using K_ = K;
		using V_ = V;
		inline static const avltree::tree_spec S_ = S;
#endif
		
		using base           = typename avltree_base::avltree<K, V, S, U>;
		using super          = typename base::tree_base;
		using KR             = typename base::KR;
		using VR             = typename base::VR;
		using data_type      = typename base::data_type;
		using iterator       = typename base::iterator;
		using node_view      = typename base::node_view;
		using node_uptr_view = typename base::node_uptr_view;
		inline static KR get_key(const data_type& data){ return data.first; }
		inline static VR get_value(const data_type& data){ return data.second; }
	public:
		using key_type      = K;
		using value_type    = V;
		inline static const avltree::tree_spec tree_spec = S;
		using with_index    = typename base::with_index;
		using with_depth    = typename base::with_depth;
		using with_value    = typename base::with_value;
		
		map(): super(){}
		template<typename K_, typename V_> bool insert(K_&& k, V_&& v){
			return this->_insert(std::forward<K_>(k), std::forward<V_>(v));
		}
		
		using iterator_key   = avltree_base::iterator_wrapper<iterator, const data_type&, KR, get_key>;
		using iterator_value = avltree_base::iterator_wrapper<iterator, const data_type&, VR, get_value>;
		
		inline iterator_key   key_begin()   const { return iterator_key(this->begin()); }
		inline iterator_value value_begin() const { return iterator_value(this->begin()); }
		
		auto reversed() const { return avltree_base::iterator_pair([this](){ return this->rbegin();      }, [this](){ return this->end(); }); }
		auto keys()     const { return avltree_base::iterator_pair([this](){ return this->key_begin();   }, [this](){ return this->end(); }); }
		auto values()   const { return avltree_base::iterator_pair([this](){ return this->value_begin(); }, [this](){ return this->end(); }); }
	
		template<typename X, X summarize(const X& a, const X& b), X identity(), X get(const typename base::data_type&) = summary::pass<data_type>> using with_summary = map<K, V, S, summary::tuple_append<U, typename summary::summarizer<K, V, S, X, summarize, identity, get>>>;
		using with_summary_key_sum    = with_summary<V, summary::add, summary::zero, summary::get_key>;
		using with_summary_key_prod   = with_summary<V, summary::mul, summary::one,  summary::get_key>;
		using with_summary_value_sum  = with_summary<V, summary::add, summary::zero, summary::get_value>;
		using with_summary_value_prod = with_summary<V, summary::mul, summary::one,  summary::get_value>;
		using with_summary_value_min  = with_summary<V, summary::min, summary::max_value, summary::get_value>;
		using with_summary_value_max  = with_summary<V, summary::max, summary::min_value, summary::get_value>;
	};
	
	template<typename V, tree_spec S = tree_spec::simple, typename U = std::tuple<>> class set: public avltree_base::avltree<V, avltree_base::empty, S, U>::tree_base{
		using K_ = V;
		using V_ = avltree_base::empty;
#ifdef AVLTREE_DEBUG_CLASS
		friend class AVLTREE_DEBUG_CLASS;
		inline static const tree_spec S_ = S;
#endif
		using base  = typename avltree_base::avltree<V, avltree_base::empty, S, U>;
		using super = typename base::tree_base;
	public:
		using value_type     = V;
		using with_index     = typename base::with_index;
		using with_depth     = typename base::with_depth;
		using with_value     = typename std::false_type;
		using node_view      = typename base::node_view;
		using node_uptr_view = typename base::node_uptr_view;
		set(): super(){}
		template<typename V_> bool insert(V_&& v){
			return this->_insert(std::forward<V_>(v));
		}
		
		template<typename X, X summarize(const X& a, const X& b), X identity(), X get(const typename base::data_type&) = summary::pass<X>> using with_summary = set<V, S, summary::tuple_append<U, typename summary::summarizer<K_, V_, S, X, summarize, identity, get>>>;
		using with_summary_sum  = with_summary<V, summary::add, summary::zero>;
		using with_summary_prod = with_summary<V, summary::mul, summary::one>;
	};
	
	template<typename V, tree_spec S = tree_spec::simple, typename U = std::tuple<>> class multiset: public avltree_base::avltree<V, avltree_base::empty, S, U>::multiset_base{
		using K_ = V;
		using V_ = avltree_base::empty;
#ifdef AVLTREE_DEBUG_CLASS
		friend class AVLTREE_DEBUG_CLASS;
		inline static const tree_spec S_ = S;
#endif
		using base  = typename avltree_base::avltree<V, avltree_base::empty, S, U>;
		using super = typename base::multiset_base;
	public:
		using value_type     = V;
		using with_index     = typename base::with_index;
		using with_depth     = typename base::with_depth;
		using with_value     = typename std::false_type;
		using node_view      = typename base::node_view;
		using node_uptr_view = typename base::node_uptr_view;
		multiset(): super(){}

		template<typename X, X summarize(const X& a, const X& b), X identity(), X get(const typename base::data_type&) = summary::pass<X>> using with_summary = multiset<V, S, summary::tuple_append<U, typename summary::summarizer<K_, V_, S, X, summarize, identity, get>>>;
		using with_summary_sum  = with_summary<V, summary::add, summary::zero>;
		using with_summary_prod = with_summary<V, summary::mul, summary::one>;
	};
	
	
}
