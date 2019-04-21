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

	std::string str() {

	}

	int getInterger() {
		
	}

	~NoConstructor() { print_destroyed(this); }
};

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

class Dog : {
public:
	Dog(const std::string &name) : Pet(name, "dog") {}
	std::string bark() const { return "Woof!"; }
};

class Rabbit : public Pet {
public:
	Rabbit(const std::string &name) : Pet(name, "parrot") {}
};

class Hamster : public Pet {
public:
	Hamster(const std::string &name) : Pet(name, "rodent") {}
};

class Chimera : public Pet {
	Chimera() : Pet("Kimmy", "chimera") {}
};

using LocalVec = std::vector<LocalType>;
using LocalVec2 = std::vector<NonLocal2>;
using LocalMap = std::unordered_map<std::string, LocalType>;

static int a = 1;

Dog function1(const Rabbit &r, Hamster *h, Chimera c)
{

}

void function2(int param1, char *param2, double &param3);