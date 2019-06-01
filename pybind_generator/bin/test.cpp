#include <string>
#include <vector>
#include <unordered_map>

struct NoConstructor {
	NoConstructor() = default;
	NoConstructor(const NoConstructor &) = default;
	NoConstructor(NoConstructor &&) = default;
	/*
		@brief: xxx
	*/	
	static NoConstructor *new_instance() {
		auto *ptr = new NoConstructor();
		printf("new instance\n");
		return ptr;
	}

	std::string str(std::string xx) const{

	}

	int get(int index) {
		
	}

	int getInterger() const {
		
	}

	~NoConstructor() { printf("destroy\n"); }
};

class B {
public:
	B(const NoConstructor *a)
	{}
};

namespace zzz
{
	double aa;
	NoConstructor bb;
	const double cc = 33;
	static int dd = 44;
	static const int ee = 55;
	namespace AAA
	{}
	namespace BBB
	{}
	namespace xxx
	{
		class Pet;
		std::string getPetName(Pet *p)
		{
			
		}
	
		class Pet {
		public:
			Pet(const std::string &name, const std::string &species)
				: m_name(name), m_species(species) {}
			/// 注释注释
			static std::string name() { return "xx"; }
			std::string species() const { return m_species; }
		
			std::string m_name;
			std::string m_species;
			static int cc;
			const int dd = 0;
			static const int ee;
			const static int ff;			
		};	
		int Pet::cc = 11111;
		const int Pet::ee = 123;
		const int Pet::ff = 321;
	} // xxx
	
} // zzz




using LocalVec = std::vector<double>;
using LocalMap = std::unordered_map<std::string, NoConstructor>;

static int a = 1;

void function1(const int &r, int *h, int c)
{

}

void function2(const int param1, char *param2=nullptr, double param3=0.0)
{
	printf("2222222\n");
}

void function2(const int num)
{
	printf("2\n");
}

enum MyEnum { EFirstEntry = 1, ESecondEntry };
enum class Enum {
	FIRST = 10,
	SECOND = 12
};

enum MEnum
{
	FIRST = 'A',
	SECOND = 'B'
};
