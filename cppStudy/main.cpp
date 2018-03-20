#include <iostream>

using namespace std;

class Vehicle
{
public:
    virtual double weight() const = 0;
    virtual void start() = 0;
    virtual Vehicle *copy() const = 0;
    virtual ~Vehicle();
};

class Automobile : public Vehicle
{
public:
    double weight() const {return m_weight;}
    void start() {}
    Vehicle *copy() const {return new Automobile(*this);}

private:
    double m_weight = 101;
};

class VehicleSurrogate {
public:
    VehicleSurrogate();
    VehicleSurrogate(const Vehicle &);
    ~VehicleSurrogate();
    VehicleSurrogate(const VehicleSurrogate &);
    VehicleSurrogate &operator = (const VehicleSurrogate &);

    double weight() const;
    void start();

private:
    Vehicle *vp;
};

VehicleSurrogate::VehicleSurrogate():
    vp(0)
{

}

VehicleSurrogate::VehicleSurrogate(const Vehicle &v)
{
    vp = v.copy();
}

VehicleSurrogate::~VehicleSurrogate()
{
    if (vp != 0)
        delete vp;
}

VehicleSurrogate::VehicleSurrogate(const VehicleSurrogate &v)
{
    vp = v.vp?v.vp->copy():0;
}

VehicleSurrogate &VehicleSurrogate::operator = (const VehicleSurrogate &v)
{
    if (this != &v) {
        delete vp;
        vp = v.vp?v.vp->copy():0;
    }
    return *this;
}

double VehicleSurrogate::weight() const
{
    if (vp == 0) {
        throw "empty vp";
    }

    return vp->weight();
}

void VehicleSurrogate::start()
{
    if (vp == 0) {
        throw "empty vp";
    }

    vp->start();
}



int main()
{
    cout << "Hello World!" << endl;
    return 0;
}
