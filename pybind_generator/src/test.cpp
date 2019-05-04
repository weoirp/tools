#include <string>

struct NoConstructor {
	NoConstructor() = default;
	NoConstructor(const NoConstructor &) = default;
	NoConstructor(NoConstructor &&) = default;
	/*
		@brief: xxx
	*/	
	static NoConstructor *new_instance() {
		auto *ptr = new NoConstructor();
		print_created(ptr, "via new_instance");
		return ptr;
	}

	std::string str(std::string xx) {

	}

	int get(int index) {
		
	}

	int getInterger() {
		
	}

	~NoConstructor() { print_destroyed(this); }
};

namespace zzz
{
	double aa;
	NoConstructor bb;
	namespace xxx
	{
		static int cc = 1;
		const int dd = 0;
		class Pet {
		public:
			Pet(const std::string &name, const std::string &species)
				: m_name(name), m_species(species) {}
			/// 注释注释
			static std::string name() const { return m_name; }
			std::string species() const { return m_species; }
		private:
			std::string m_name;
			std::string m_species;
		};		
	} // xxx
	
} // zzz




using LocalVec = std::vector<VariableData>;
using LocalVec2 = std::vector<FunctionData>;
using LocalMap = std::unordered_map<std::string, NameSpaceData>;

static int a = 1;

NameSpaceData function1(const EnumData &r, AccessData *h, ClassData c)
{

}

void function2(int param1, char *param2, double &param3);

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