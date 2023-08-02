#include <map>
#include <set>
#include <list>
#include <cmath>
#include <ctime>
#include <deque>
#include <queue>
#include <stack>
#include <string>
#include <bitset>
#include <cstdio>
#include <limits>
#include <vector>
#include <climits>
#include <cstring>
#include <cstdlib>
#include <fstream>
//#include <numeric>
#include <functional>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <variant>
#include <memory>
#include <queue>
#include <functional>
#include <type_traits>
using std::runtime_error;
using std::string;
using std::stoi;
using std::vector;
using std::variant;
using std::map;
using std::unordered_map;
using std::cin;
using std::istringstream;
using std::shared_ptr;
using std::make_shared;
using std::pair;
using std::to_string;
using std::cout;
using std::endl;
using std::function;
using std::get;
using std::greater;
using std::priority_queue;
using std::less;

//https://github.com/arximboldi/immer
// lines could be outdated but should be close, refer to function name in such case
// todo wish: state map isn't great. the main problem are the transactions. would be nice to to come back to and redesign
// todo wish: make everything immutable rather than mutating state in place would have allowed for safe const reference accessing, possibly making
// things faster

const string BUY = "BUY";
const string SELL = "SELL";
const string CANCEL = "CANCEL";
const string MODIFY = "MODIFY";
const string PRINT = "PRINT";
const string OUTSTANDING_BUYS = "outstanding_buys";
const string OUTSTANDING_SELLS = "outstanding_sells";
const string TRADES_COMPLETED = "trades_completed";
const string ORDER_HISTORY = "order_history";
const string IOC = "IOC";

template <class R, class A, class... B>
auto curry(function<R(A, B...)> f) {
	if constexpr (sizeof...(B) > 0) {
		return [=](A a) {
			return curry(function<R(B...)>{bind_front(f, a)});
		};
	}
	else {
		return f;
	}
}


template<typename V>
void show(V value) {
	cout << value << endl;
}

template<typename C, typename F>
void print_collection(C collection, F unwrapper) {
	for (const auto& i : collection) {
		cout << "I: " << unwrapper(i) << endl;
	}
	show("------------");
}

struct order {
	// makes type polymorphic
	virtual ~order() {};
	string operation;
};

struct transaction_order :public order {

	transaction_order() {
	}
	transaction_order(const vector<string>& transaction_order) {
		operation = transaction_order[0];
		type = transaction_order[1];
		price = stoi(transaction_order[2]);
		quantity = stoi(transaction_order[3]);
		id = transaction_order[4];
	}
	string type;
	int price;
	int quantity;
	string id;
};

struct cancel_order :public order {
	cancel_order() {
		string message = "Attempted to initalize cancnel order with no parameters. ";
		throw runtime_error(message);
	}
	cancel_order(const vector<string>& cancel_order) {
		operation = cancel_order[0];
		id = cancel_order[1];
	}
	string id;
};

struct modify_order :public order {
	modify_order() {
		string message = "Attempted to initalize modify order with no parameters. ";
		throw runtime_error(message);
	}
	modify_order(const vector<string>& transaction_order) {
		operation = transaction_order[0];
		id = transaction_order[1];
		action = transaction_order[2];
		price = stoi(transaction_order[3]);
		quantity = stoi(transaction_order[4]);
	}
	string id;
	string action;
	int price;
	int quantity;
};

struct print_order :public order {
	print_order(vector<string> _) {
		(void) _;
		operation = PRINT;
	}
	print_order() {
		operation = PRINT;
	}
};

struct trade_order :public order {
	trade_order() {
		string message = "Attempted to initalize a trade order with no parameters. ";
		throw runtime_error(message);
	}
	trade_order(transaction_order higher_quantity_order, transaction_order lower_quantity_order) {
		operation = "TRADE";
		higher_id = higher_quantity_order.id;
		higher_order_price = higher_quantity_order.price;
		higher_quantity = higher_quantity_order.quantity;
		lower_id = lower_quantity_order.id;
		lower_order_price = lower_quantity_order.price;
		lower_quantity = lower_quantity_order.quantity;
	}
	string higher_id;
	int higher_order_price;
	int higher_quantity;
	string lower_id;
	int lower_order_price;
	int lower_quantity;
};

using storage_types = variant<
	transaction_order,
	map<int, map<int, transaction_order>>,
	vector<trade_order>,
	map<string, transaction_order>>;
template <typename V>
using matching_engine = unordered_map<string, V>;

template <typename T, typename I, typename F>
I reduce(const T& list, I initial_value, F func) {
	I result = initial_value;
	for (const auto& item : list) {
		result = func(result, item);
	}
	return result;
}

vector<int> range(int n) {
	vector<int> result;
	for (int i = 0; i <= n; ++i) {
		result.push_back(i);
	}
	return result;
}

template<typename K, typename V, typename F>
map<K, V> filter(map<K, V> unfiltered_map, F criteria) {
	map<K, V> filtered_map;
	for (const auto& pair : unfiltered_map) {
		if (criteria(pair)) {
			filtered_map.emplace(pair.first, pair.second);
		}
	}
	return filtered_map;

}

template <typename K, typename V>
bool contains(map<K, V> map, K key) { // already implmented in cpp 20 prbs
	// replace with x[y] = z?
	return map.find(key) != map.end();
}

template <typename K, typename V>
map<K, V> assoc(map<K, V> collection, const K key, const V value) {
	if (contains(collection, key)) {
		collection[key] = value;
	}
	else {
		collection.emplace(key, value);
	}
	return collection;
}


template<typename K, typename V>
void print_map(map<K, V> map) {
	for (const auto& pair : map)
		cout << "k: " << pair.first << " | " << "v: " << pair.second << endl;
}

vector<string> get_input_data() {
	vector<string> orders;
	string order;
	while (getline(cin, order) && !order.empty()) {
		orders.push_back(order);
	}
	return orders;
}

vector<string> parse_order(string order_line) {
	vector<string> parsed_order;
	istringstream tokens(order_line);
	string order_argument;
	while (tokens >> order_argument) {
		parsed_order.push_back(order_argument);
	}
	return parsed_order;
}

vector<vector<string>> get_orders_from_input() {
	auto input_data = get_input_data();
	vector<vector<string>> list_of_orders;
	return reduce(input_data, list_of_orders, [](auto orders, auto order_line) {
		orders.push_back(parse_order(order_line));
		return orders;
		});
}

// returns any kind of struct
shared_ptr<order> match_order(vector<string> order_vec) {
	auto operation = order_vec[0]; // BUY
	if (operation == BUY) {
		// buy -> op, type, price, quanity, id
		return make_shared<transaction_order>(order_vec);
	}
	else if (operation == SELL) {
		// sell -> same
		return make_shared<transaction_order>(order_vec);
	}
	else if (operation == CANCEL) {
		// cancel -> op, id
		return make_shared<cancel_order>(order_vec);
	}
	else if (operation == MODIFY) {
		return make_shared<modify_order>(order_vec);
	}
	else { // assumes: operation == PRINT
		return make_shared<print_order>();
	}
}

vector<shared_ptr<order>> match_order_to_struct(vector<vector<string>> order_matrix) {
	vector<shared_ptr<order>> order_list;
	return reduce(order_matrix, order_list, [](auto list_of_orders, auto order_vec) {
		auto order_as_struct = match_order(order_vec);
		list_of_orders.push_back(order_as_struct);
		return list_of_orders;
		});
}

template <typename V>
matching_engine<V> build_state_map() {
	matching_engine<storage_types> new_state_map;
	// if seller is seeker -> start with highest buyer
	map<int, map<int, transaction_order>> outstanding_buys;
	// if buyer is seeker -> start with lowest seller (default)
	map<int, map<int, transaction_order>> outstanding_sells;
	vector<trade_order> trades_completed;
	map<string, transaction_order> order_history;
	transaction_order most_recent_seeker;
	new_state_map.emplace(OUTSTANDING_BUYS, outstanding_buys);
	new_state_map.emplace(OUTSTANDING_SELLS, outstanding_sells);
	new_state_map.emplace(TRADES_COMPLETED, trades_completed);
	new_state_map.emplace(ORDER_HISTORY, order_history);
	return new_state_map;
}

template <typename V>
map<int, map<int, transaction_order>> get_transactions(matching_engine<V> state, string operation) {
	auto transaction_type = operation == BUY ? OUTSTANDING_BUYS : OUTSTANDING_SELLS;
	auto relevant_transactions = get<map<int, map<int, transaction_order>>>(state[transaction_type]);
	return relevant_transactions;
}

template <typename V>
transaction_order get_transaction(matching_engine<V> state, string operation, int price, int index) {
	return get_transactions(state, operation)[price][index];
}

template <typename V>
map<string, transaction_order> get_order_history(matching_engine<V> state) {
	return get<map<string, transaction_order>>(state[ORDER_HISTORY]);
}

template <typename V, typename F>
matching_engine<V> update_transaction(matching_engine<V> state, string operation, int price, int index, F updater) {
	auto transaction_type = operation == BUY ? OUTSTANDING_BUYS : OUTSTANDING_SELLS;
	auto transaction = get_transactions(state, operation)[price][index];
	auto updated_transaction = updater(index, transaction);
	get<map<int, map<int, transaction_order>>>(state[transaction_type])[price][index] = updated_transaction;
	return state;
}

template <typename V, typename F>
matching_engine<V> update_transactions(matching_engine<V> state, string operation, F updater) {
	map<int, map<int, transaction_order>> relevant_transactions = get_transactions(state, operation);
	auto updated_transactions = updater(relevant_transactions);
	auto transaction_type = operation == BUY ? OUTSTANDING_BUYS : OUTSTANDING_SELLS;
	state[transaction_type] = updated_transactions;
	return state;
}

template <typename V>
matching_engine<V> add_trade(matching_engine<V> state, trade_order trade) {
	vector<trade_order>& trades_made = get<vector<trade_order>>(state[TRADES_COMPLETED]);
	trades_made.push_back(trade);
	return state;
}

template <typename V>
matching_engine<V> update_quantity(matching_engine<V> state, string operation, int price, int index, int quantity) {
	auto update_quantity = [&quantity](int index, transaction_order order) {
		order.quantity = quantity;
		return order;
	};											 
	state = update_transaction<V>(state, operation, price, index, update_quantity);
	return state;
}

map<int, transaction_order> adjust_transaction_keys(map<int, transaction_order> relevant_price_orders_to_update, int index) {
	auto index_iterator = relevant_price_orders_to_update.find(index);
	auto entries_to_keep = map<int, transaction_order>(relevant_price_orders_to_update.begin(), --index_iterator);
	auto entries_to_update = map<int, transaction_order>(relevant_price_orders_to_update.lower_bound(index), relevant_price_orders_to_update.end());
	map<int, transaction_order> updated_entries = reduce(entries_to_update, map<int, transaction_order>{}, [](map<int, transaction_order> new_map, pair<int, transaction_order> old_pair) {
		new_map.emplace(old_pair.first - 1, old_pair.second);
		return new_map;
		});
	for (const auto& updated_pair : updated_entries) {
		entries_to_keep.insert(updated_pair);
	}
	return entries_to_keep;
}

template <typename V>
matching_engine<V> adjust_price_keys(matching_engine<V> state, string operation, int price, int index) {
	map<int, transaction_order> relevant_price_orders_to_update = get_transactions(state, operation)[price];
	auto index_iterator = relevant_price_orders_to_update.find(index);
	auto entries_to_keep = adjust_transaction_keys(relevant_price_orders_to_update, index);
	state = update_transactions<V>(state, operation, [&price, &entries_to_keep](map<int, map<int, transaction_order>> transactions) {
		transactions[price] = entries_to_keep;
		return transactions;
		});
	return state;
}

template<typename S>
map<int, map<int, transaction_order>, S> remove_relevant_asker(map<int, map<int, transaction_order>, S> remaining_relevant_askers, int price, int index) {
	auto adjustment_needed = remaining_relevant_askers[price].size() > index;
	remaining_relevant_askers[price].erase(index);
	auto no_more_orders_at_price = remaining_relevant_askers[price].empty();
	if (no_more_orders_at_price) {
		remaining_relevant_askers.erase(price);
	}
	else if (adjustment_needed) {
		remaining_relevant_askers[price] = adjust_transaction_keys(remaining_relevant_askers[price], index);
	}
	return remaining_relevant_askers;
}

template <typename V>
matching_engine<V> remove_transaction(matching_engine<V> state, string operation, int price, int index) {
	auto transactions = get_transactions(state, operation);
	auto may_have_keys_that_need_adjusted = index < transactions.size()-1;
	transactions[price].erase(index);
	auto no_more_orders_at_price = transactions[price].empty();
	auto adjustment_needed = may_have_keys_that_need_adjusted && !no_more_orders_at_price;
	if (no_more_orders_at_price) {
		state = update_transactions<V>(state, operation, [&price](auto transactions) {
			transactions.erase(price);
			return transactions;
			});
	}
	else {
		state = update_transactions<V>(state, operation, [&transactions](auto _) {
			return transactions;
			});
	}
	if (adjustment_needed) {
		state = adjust_price_keys<V>(state, operation, price, index);
	}

	return state;
}

template <typename V>
matching_engine<V> perform_disparate_trade(matching_engine<V> state, pair<int, transaction_order> higher_quantity_order_pair, pair<int, transaction_order> lower_quantity_order_pair) {
	auto higher_quantity_order = higher_quantity_order_pair.second;
	auto lower_quantity_order = lower_quantity_order_pair.second;
	trade_order new_trade{ higher_quantity_order, lower_quantity_order };
	auto new_quantity = higher_quantity_order.quantity - lower_quantity_order.quantity;
	//& = access by reference
	state = add_trade<V>(state, new_trade);
	state = remove_transaction<V>(state, lower_quantity_order.operation, lower_quantity_order.price, lower_quantity_order_pair.first);
	state = update_quantity<V>(state, higher_quantity_order.operation, higher_quantity_order.price, higher_quantity_order_pair.first, new_quantity);
	return state;
}

template <typename V>
matching_engine<V> perform_equal_trade(matching_engine<V> state, pair<int, transaction_order> seeker_pair, pair<int, transaction_order> asker_pair) {
	auto seeker = seeker_pair.second;
	auto asker = asker_pair.second;
	trade_order new_trade{ seeker, asker };
	state = add_trade<V>(state, new_trade);
	state = remove_transaction<V>(state, seeker.operation, seeker.price, seeker_pair.first);
	state = remove_transaction<V>(state, asker.operation, asker.price, asker_pair.first);
	return state;
}

// if seller price > higher price, no transaction is made
map<int, map<int, transaction_order>> find_matchers(string operation, int price, map<int, map<int, transaction_order>> traders) {
	return operation == BUY
		? map<int, map<int, transaction_order>>(traders.begin(), traders.upper_bound(price))
		: map<int, map<int, transaction_order>>(traders.lower_bound(price), traders.end());
}

template <typename K, typename V>
matching_engine<V> update_order_history(matching_engine<V> state, transaction_order order) {
	map<string, transaction_order> order_history = get_order_history(state);
	order_history = assoc<string, transaction_order>(order_history, order.id, order);
	state[ORDER_HISTORY] = order_history;
	return state;
}

template <typename V>
matching_engine<V> add_outstanding_seeker(matching_engine<V> state, transaction_order seeker) {
	auto price = seeker.price;
	map<int, map<int, transaction_order>> relevant_transactions = get_transactions(state, seeker.operation);
	auto transactions_at_price = relevant_transactions.find(price);
	map<int, transaction_order> transactions;
	int index;
	if (transactions_at_price != relevant_transactions.end()) {
		transactions = transactions_at_price->second;
		index = static_cast<int>(transactions.size());
	}
	else {
		index = 0;
	}
	transactions.emplace(index, seeker);
	relevant_transactions[price] = transactions;
	state = update_transactions<V>(state, seeker.operation, [&relevant_transactions](auto _) {
		return relevant_transactions;
		});
	return state;
};

void print_trade(trade_order trade) {
	cout << "TRADE ";
	cout << trade.lower_id << " " << trade.lower_order_price << " " << trade.lower_quantity << " ";
	cout << trade.higher_id << " " << trade.higher_order_price << " " << trade.lower_quantity << " ";
	cout << endl;
}

template <typename V>
int get_seeker_index(matching_engine<V> state, string seeker_operation, int seeker_price) {
	auto seeker_transactions = get_transactions(state, seeker_operation);
	auto seeker_transactions_at_price = seeker_transactions[seeker_price];
	auto relevant_seeker_pointer = seeker_transactions_at_price.rbegin();
	auto seeker_index = relevant_seeker_pointer->first;
	return seeker_index;
}

template<typename V, typename S>
matching_engine<V> process_transaction(transaction_order seeker_order, matching_engine<V> state, map<int, map<int, transaction_order>, S> ordered_relevant_askers) {
	auto seeker_operation = seeker_order.operation;
	auto seeker_price = seeker_order.price;
	state = add_outstanding_seeker<V>(state, seeker_order);
	auto seeker_index = get_seeker_index<V>(state, seeker_operation, seeker_price);
	auto id = seeker_order.id;
	auto seeker_quantity = seeker_order.quantity;
	//state = update_current_seeker(state, seeker_order);
	state = update_order_history<V>(state, seeker_order);
	int remaining_quantity = seeker_quantity;

	map<int, map<int, transaction_order>, S> remaining_relevant_askers = ordered_relevant_askers;
	while (remaining_quantity > 0 && !remaining_relevant_askers.empty()) {
		auto askers_at_price = remaining_relevant_askers.begin()->second;
		const pair<int, transaction_order>& most_relevant_asker_pair = *askers_at_price.begin();
		transaction_order most_relevant_asker = most_relevant_asker_pair.second;
		auto current_seeker = get_transaction(state, seeker_operation, seeker_price, seeker_index);
		auto asking_quantity = most_relevant_asker.quantity;
		auto seeker_is_higher = remaining_quantity > asking_quantity;
		auto equal_quantities = remaining_quantity == asking_quantity;
		if (seeker_is_higher) {
			state = perform_disparate_trade<V>(state, pair<int, transaction_order>{seeker_index, current_seeker}, most_relevant_asker_pair);
			remaining_quantity = remaining_quantity - asking_quantity;
			remaining_relevant_askers = remove_relevant_asker(remaining_relevant_askers, most_relevant_asker.price, most_relevant_asker_pair.first);
		}
		else if (equal_quantities) {
			auto seeker_pair = pair<int, transaction_order>{ seeker_index, current_seeker };
			state = perform_equal_trade<V>(state, seeker_pair, most_relevant_asker_pair);
			remaining_quantity = 0;
		}
		else {
			state = perform_disparate_trade<V>(state, most_relevant_asker_pair, pair<int, transaction_order>{ seeker_index, current_seeker });
			remaining_quantity = 0;
		}
		auto trade = get<vector<trade_order>>(state[TRADES_COMPLETED]).back();
		print_trade(trade);
		state = update_order_history<V>(state, most_relevant_asker);
		state = update_order_history<V>(state, current_seeker);
	}

	auto type = seeker_order.type;
	auto seeker_is_outstanding = remaining_quantity > 0;
	if (seeker_is_outstanding && type == IOC) {
		state = remove_transaction<V>(state, seeker_operation, seeker_price, seeker_index);
	}
	return state;
}

template <typename V>
matching_engine<V> handle_buy_tansaction(matching_engine<V> state, map<int, map<int, transaction_order>> relevant_askers, transaction_order seeker_order) {
	map<int, map<int, transaction_order>, less<int>> ordered_relevant_askers;
	for (const auto& pair : relevant_askers) {
		ordered_relevant_askers.emplace(pair.first, pair.second);
	}

	
	return process_transaction(seeker_order, state, ordered_relevant_askers);

}

template <typename V>
matching_engine<V> handle_sell_transaction(matching_engine<V> state, map<int, map<int, transaction_order>> relevant_askers, transaction_order seeker_order) {
	map<int, map<int, transaction_order>, greater<int>> ordered_relevant_askers;
	for (const auto& pair : relevant_askers) {
		ordered_relevant_askers.emplace(pair.first, pair.second);
	}
	return process_transaction(seeker_order, state, ordered_relevant_askers);
}

template <typename V>
matching_engine<V> process_transaction_order(string seeker_operation, matching_engine<V> state, transaction_order seeker_order) {
	auto seeker_price = seeker_order.price;
	auto asker_operation = seeker_operation == BUY ? SELL : BUY;
	map<int, map<int, transaction_order>> available_askers = get_transactions(state, asker_operation);
	map<int, map<int, transaction_order>> relevant_askers = find_matchers(seeker_operation, seeker_price, available_askers);
	return seeker_operation == BUY 
		? handle_buy_tansaction(state, relevant_askers, seeker_order)
		: handle_sell_transaction(state, relevant_askers, seeker_order);
}

template <typename C, typename F>
auto search(const C& collection, F criteria) {
	return find_if(collection.begin(), collection.end(), criteria);
}

template <typename V>
auto find_order_by_id(const matching_engine<V>& state, string operation, int price, string id) {
	map<int, transaction_order> orders_to_check = get_transactions(state, operation)[price];
	auto result = search<map<int, transaction_order>>(orders_to_check, [&id](const pair<int, transaction_order>& order_pair) {
		return order_pair.second.id == id;
		});
	if (result != orders_to_check.end()) {
		return result->first;
	}
	return  -1; // orders_to_check.end(); // todo bonus: find better answer than this
}

template <typename V>
matching_engine<V> cancel_order_and_update_history(matching_engine<V> state, string operation, int stale_price, int order_position, string order_id) {
	transaction_order order_to_cancel = get_transaction(state, operation, stale_price, order_position);
	state = remove_transaction<V>(state, operation, order_to_cancel.price, order_position);
	get<map<string, transaction_order>>(state[ORDER_HISTORY]).erase(order_id);
	return state;
}

template <typename V>
matching_engine<V> process_cancel_order(matching_engine<V> state, cancel_order requested_order_to_cancel) {
	map<string, transaction_order> order_history = get_order_history(state);
	transaction_order order_to_cancel_info = order_history[requested_order_to_cancel.id];
	auto operation = order_to_cancel_info.operation;
	auto cancel_price = order_to_cancel_info.price;
	auto order_position = find_order_by_id(state, operation, cancel_price, order_to_cancel_info.id);
	auto found_order = order_position != -1;
	if (found_order) {
		state = cancel_order_and_update_history<V>(state, operation, cancel_price, order_position, requested_order_to_cancel.id);
	}
	return state;
}

template <typename V>
matching_engine<V> process_modify_order(matching_engine<V> state, modify_order modify_info) {
	transaction_order stale_order = get_order_history(state)[modify_info.id];
	auto operation = stale_order.operation;
	auto id = stale_order.id;
	auto price = stale_order.price;
	auto stale_order_index = find_order_by_id(state, operation, price, id); // does not handle no order found rn
	transaction_order fresh_order = get_transactions(state, operation)[price][stale_order_index];
	auto same_operation = fresh_order.operation == modify_info.action;
	auto same_price = price == modify_info.price;
	auto same_transactions = same_operation && same_price;
	auto same_operation_different_price = same_operation && !same_price;
	fresh_order.operation = modify_info.action;
	fresh_order.price = modify_info.price;
	fresh_order.quantity = modify_info.quantity;
	if (same_transactions) {
		state = remove_transaction<V>(state, fresh_order.operation, fresh_order.price, stale_order_index);
		state = add_outstanding_seeker<V>(state, fresh_order);
	}
	else if (same_operation_different_price) {
		state = remove_transaction<V>(state, fresh_order.operation, stale_order.price, stale_order_index);
		state = add_outstanding_seeker<V>(state, fresh_order);
	}
	else { // switch sides, should trade
		state = remove_transaction<V>(state, stale_order.operation, stale_order.price, stale_order_index);
		state = process_transaction_order<V>(fresh_order.operation, state, fresh_order);
	}
	return state;
}

vector<string> build_trade_string(
	vector<string> trade_list,
	pair<int, int> quantity_at_price) {
	auto price = quantity_at_price.first;
	auto quantity = quantity_at_price.second;
	auto trader_string = to_string(price) + " " + to_string(quantity);
	trade_list.push_back(trader_string);
	return trade_list;
}

void print_lines(vector<string> lines) {
	for (const string& line : lines) {
		cout << line << endl;
	}
}

map<int, int> build_price_quantity_map(map<int, int> price_to_quantity, pair<int, map<int, transaction_order>> price_to_orders) {
	int quantity = reduce(price_to_orders.second, 0, [](auto quantity, pair<int, transaction_order> order) { return quantity + order.second.quantity; });
	price_to_quantity.emplace(price_to_orders.first, quantity);
	return price_to_quantity;
};

template <typename V>
void process_print_order(matching_engine<V> state) {
	map<int, map<int, transaction_order>> outstanding_buys = get_transactions(state, BUY);
	map<int, map<int, transaction_order>> outstanding_sells = get_transactions(state, SELL);
	map<int, int> buys_to_print = reduce(outstanding_buys, map<int, int>{}, build_price_quantity_map);
	map<int, int> sells_to_print = reduce(outstanding_sells, map<int, int>{}, build_price_quantity_map);
	auto buys_as_trade_strings = reduce(buys_to_print, vector<string>{"BUY:"}, build_trade_string);
	auto sells_as_trade_strings = reduce(sells_to_print, vector<string>{"SELL: "}, build_trade_string);
	print_lines(sells_as_trade_strings);
	print_lines(buys_as_trade_strings);
}

void print_transaction_order(transaction_order order) {
	show("printing transaction contents...");
	show(order.operation);
	show(order.type);
	show(order.price);
	show(order.quantity);
	show(order.id);
	show("------------");
}

template <typename V>
void print_trades(matching_engine<V> state) {
	auto trades = get<vector<trade_order>>(state[TRADES_COMPLETED]);
	for (const auto& trade : trades) {
		print_trade(trade);
	}

}

const string DEFAULT = "default";
				 
// todo: concepts
template <typename R, typename K, typename I, typename F>
auto match() {
	auto uncurried_match = function<R(K, I, map<K, F>)>{ [](K key, I input, map<K, F> options) {
		auto operation = contains(options, key) ? options[key] : options[DEFAULT];
		return operation(input);
		}};
	return curry(uncurried_match);
}


template <typename R, typename I, typename F>
auto match_on_operation(string operation) {
	return match<R, string, I, F>()(operation);
}

using order_type = variant<transaction_order, cancel_order, modify_order, order>;
auto discern_order_type(shared_ptr<order> current_order, string operation) {
	map<string, function<order_type(shared_ptr<order>)>> casting_options = {
		{ BUY, [](auto current_order) { return dynamic_cast<const transaction_order&>(*current_order); }},
		{ SELL, [](auto current_order) { return dynamic_cast<const transaction_order&>(*current_order); }},
		{ CANCEL, [](auto current_order) { return dynamic_cast<const cancel_order&>(*current_order); }},
		{ MODIFY, [](auto current_order) { return dynamic_cast<const modify_order&>(*current_order); } },
		{ DEFAULT, [](auto current_order) { return dynamic_cast<const order&>(*current_order); } } };
	auto act_on_operation = match_on_operation<order_type, shared_ptr<order>, function<order_type(shared_ptr<order>)>>(operation);
	return act_on_operation(current_order)(casting_options);
	}

template <typename V>
auto match_process(string operation, matching_engine<V> state, order_type casted_order) {
	map<string, function<matching_engine<V>(order_type)>> casting_options = {
	{ BUY, [&state](auto casted_order) { return process_transaction_order(BUY, state, get<transaction_order>(casted_order)); }},
	{ SELL, [&state](auto casted_order) { return process_transaction_order(SELL, state, get<transaction_order>(casted_order)); }},
	{ CANCEL, [&state](auto casted_order) { return process_cancel_order(state, get<cancel_order>(casted_order)); }},
	{ MODIFY, [&state](auto casted_order) {
			auto modify_order_info = get<modify_order>(casted_order);
			auto type = get_order_history(state)[modify_order_info.id].type;
			return type == IOC ? state : process_modify_order(state, modify_order_info);
		}},
	{ DEFAULT, [&state](auto _) { return state; } } };
	auto act_on_operation = match_on_operation<matching_engine<V>, order_type, function<matching_engine<V>(order_type)>>(operation);
	return act_on_operation(casted_order)(casting_options);
}

template <typename V>
matching_engine<V> process_order(matching_engine<V> state, shared_ptr<order> current_order) {
	auto operation = current_order->operation;
	auto casted_order = discern_order_type(current_order, operation);
	state = match_process(operation, state, casted_order);
	return state;
	}

//Tests	 // todo bonus: add more test
template <typename V>
void test_adding_order() {
	show("Testing adding order...");
	transaction_order test_order{ vector<string>{"BUY", "GFD", "1000", "10", "ORDER1"} };
	auto test = build_state_map<storage_types>();
	auto transactions = get<map<int, map<int, transaction_order>>>(test[OUTSTANDING_BUYS]);
	transactions[0].emplace(0, test_order);
	show("trying direct access...");
	show(get_transactions(test, OUTSTANDING_BUYS)[1000][0].price);
	show("after direct");
	auto buyer = get_transactions(test, BUY)[1000];
	show("Printing prices...");
	print_collection(buyer, [](pair<int, transaction_order> o) { return o.second.price; });
}


auto validate_transaction_order(vector<vector<string>> valid_orders, vector<string> unvalidated_order) {
	auto valid_size = unvalidated_order.size() == 5;
	auto get = [&valid_size](auto v, auto op) { return valid_size && op(v);  };
	auto valid_price = get(unvalidated_order[2], [](auto v) { return stoi(v) > 0; });
	auto valid_quantity = get(unvalidated_order[3], [](auto v) { return stoi(v) > 0; });
	auto valid_id = get(unvalidated_order[4], [](auto v) { return !v.empty(); });
	if (valid_size && valid_price && valid_quantity && valid_id) {
		valid_orders.push_back(unvalidated_order);
	}
	return valid_orders;
}

auto validate_cancel_order(vector<vector<string>> valid_orders, vector<string> unvalidated_order) {
	auto valid_size = unvalidated_order.size() == 2;
	auto get = [&valid_size](auto v, auto op) { return valid_size && op(v);  };
	auto valid_id = get(unvalidated_order[1], [](auto v) { return !v.empty(); });
	if (valid_id) {
		valid_orders.push_back(unvalidated_order);
	}
	return valid_orders;
}

auto validate_modify_order(vector<vector<string>> valid_orders, vector<string> unvalidated_order) {
	auto valid_size = unvalidated_order.size() == 5;
	auto get = [&valid_size](auto v, auto op) { return valid_size && op(v);  };
	auto valid_price = get(unvalidated_order[3], [](auto v) { return stoi(v) > 0; });
	auto valid_quantity = get(unvalidated_order[4], [](auto v) { return stoi(v) > 0; });
	auto valid_id = get(unvalidated_order[1], [](auto v) { return !v.empty(); });
	if (valid_size && valid_price && valid_quantity && valid_id) {
		valid_orders.push_back(unvalidated_order);
	}
	return valid_orders;
}

// commentary: we can check for a lot of things here. for example we could also check that
// for buy and sell, we're given either IOC or GFD. sticking to what the instructions ask to verify
auto match_validator(string operation, vector<vector<string>> valid_orders, vector<string> unvalidated_order) {
	map<string, function<vector<vector<string>>(vector<string>)>> validation_options = {
	{ BUY, [&valid_orders](auto unvalidated_order) {
			return validate_transaction_order(valid_orders, unvalidated_order);
	}},
	{ SELL, [&valid_orders](auto unvalidated_order) { 
			return validate_transaction_order(valid_orders, unvalidated_order);
	}},
	{ CANCEL, [&valid_orders](auto unvalidated_order) { 
			return validate_cancel_order(valid_orders, unvalidated_order);
	}},
	{ MODIFY, [&valid_orders](auto unvalidated_order) {
			return validate_modify_order(valid_orders, unvalidated_order);
	}},
	{ DEFAULT, [&valid_orders](auto unvalidated_order) { valid_orders.push_back(unvalidated_order);  return valid_orders; } } };
	auto act_on_operation = match_on_operation<vector<vector<string>>, vector<string>, function<vector<vector<string>>(vector<string>)>>(operation);
	return act_on_operation(unvalidated_order)(validation_options);
}

auto validate_orders(vector<vector<string>> valid_orders, vector<string> unvalidated_order) {
	string operation = unvalidated_order[0];
	auto result = match_validator(operation, valid_orders, unvalidated_order);
	return result;
}



auto example_function() {
	return curry(function<int(int, int)>{[](int a, int b) {
		return a + b;
		}});
}


int main() {
	auto matrix_of_orders = get_orders_from_input();
	auto matrix_of_valid_orders = reduce(matrix_of_orders, vector<vector<string>>{}, validate_orders);
	auto list_of_orders = match_order_to_struct(matrix_of_valid_orders);
	auto state_map = build_state_map<storage_types>();
	auto final_machine_state = reduce(list_of_orders, state_map, [](auto state_map, const shared_ptr<order> current_order) {
		state_map = process_order(state_map, current_order);
		return state_map;
		});
	auto input = 69; // input from external
	auto weight = input * 3; /// business logic
	auto grades = { 80, 89, 90, 90, 91, 101 };
	// sum weight to grades, return result;
	// for loop, add numbers, put in new vector
	// some sort of map reduce 

	auto a1 = example_function(); // a1 hold a function that accepts parameters in the from int -> int -> int + int
	auto add_to_weight = example_function()(weight); // <- closure with weight
	// map takes a collection -> [ 1 2 3 ] -> applies a function to all elements in collection and replaces the result
	//e.g. map([1 2 3], x -> x + 1) -> [2 3 4] 
 // 	auto result = map(grades, add_to_weight);
	auto a2 = a1(1)(1);

	//std::cout << a2 << std::endl;
	return 0;
}
