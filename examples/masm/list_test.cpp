#include <list>
#include <iostream>



void print(const std::list<int> ls) {
	for(const auto& x : ls) std::cout << x << ", ";
	std::cout << std::endl;
}
int main() {

	const std::list<int> ls1 {1, 2, 3, 4, 5};
	const std::list<int> ls2(:, ls1.cend());
	print(ls1);
	print(ls2);

}
