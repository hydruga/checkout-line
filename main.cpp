#include <iostream>
//#include <memory>
#include <vector>
#include <ctime>
#include <chrono>
#include <thread>
#include <algorithm>


class checkout_line {
private:
    struct Node{
        char customer;
        Node *next;
        std::time_t nodetime ; //std::time_t is measured in seconds


        Node(char cust = '*', Node *nex = nullptr, std::time_t n_time = std::time(nullptr))
        {
            customer=cust;
            next= nex;
            nodetime = n_time; //timestamp is created
        }

    };
    //std::mutex g_Mutex;
    Node *front_line;
    Node *back_line;
    int line_count;
    std::string name;


public:
    checkout_line(std::string name){
        this->name = name;
        front_line = nullptr,
        back_line = nullptr,
        line_count = 0;
    }
    //std::future<void> result;
    ~checkout_line();
    void push();
    void display();
    bool empty() const;
    int getLineCount() const;
    void checkout();
    std::string get_name();

}; //end of checkout_line class
std::string checkout_line::get_name() { return this->name; }
void checkout_line::push(){

    if(!front_line)
    {
        back_line = new Node();
        front_line = back_line;
    }
    else
    {
        back_line->next = new Node();
        back_line = back_line->next;
    }
    line_count++;
//    result = std::async(std::launch::async, &checkout_line::checkout, this) ; //async launch policy for high level concurrency
//    //ref https://stackoverflow.com/questions/30195756/no-matching-function-for-call-to-asyncstdlaunch-unresolved-overloaded-fun (previously broken)
//    //async returns a future object
//    result.get();//get makes main thread wait
}

void checkout_line::checkout() {
    if (empty()) { return; }
    else {
            Node *front = front_line;
            int time_val = int(difftime(std::time(nullptr), front->nodetime)); //get current sys time
            std::cout << this->name << " at val time " << time_val << std::endl;
            if (time_val >= 6) {
                //g_Mutex.lock();
                front_line = front_line->next;
                if (empty()) {
                    back_line = nullptr;
                }
                else { front_line->nodetime = std::time(nullptr); }
                delete front;
                line_count--;
            }
            //g_Mutex.unlock();
    }
} //end checkout

void checkout_line::display(){
    Node *nPtr = front_line;
    if(empty()) return;
    while(nPtr){
        std::cout << nPtr->customer << " --> ";
        nPtr = nPtr->next;
    }
}//end display()

bool checkout_line::empty() const{
    return front_line == nullptr;
}//end empty()

int checkout_line::getLineCount() const {
    return line_count;
}//end getLineCount()

checkout_line::~checkout_line()
{
    Node *clear = front_line;
    while(clear != nullptr)
    {
        Node *garbage = clear;
        clear = clear->next;
        delete garbage;
        line_count--;
    }
} //end destructor

class checkout_manager{
private:
    std::time_t manager_time;
public:
    void checkout_shoppers(std::vector<checkout_line*> &);
    void get_display(std::vector<checkout_line*> &) const;
    int get_next_line(std::vector<checkout_line*> &) const;
    void closing_time(std::vector<checkout_line*>&);
    checkout_manager() {
        std::vector<checkout_line *> lines;
        checkout_line line0("line_0"), line1("line_1"), line2("line_2"), line3("line_3"),
        line4("line_4"), line5("line_5"), line6("line_6"), line7("line_7"), line8("line_8");
        //initialize checkout lines and put into vector
        lines.push_back(&line0);
        lines.push_back(&line1);
        lines.push_back(&line2);
        lines.push_back(&line3);
        lines.push_back(&line4);
        lines.push_back(&line5);
        lines.push_back(&line6);
        lines.push_back(&line7);
        lines.push_back(&line8);
        manager_time = std::time(nullptr); //get current time, this will track everything

        //preload checkout customers to test 6 second delay for processing each
        lines.at(1)->push();
        lines.at(1)->push();
        lines.at(1)->push();
        lines.at(4)->push();
        lines.at(4)->push();
        lines.at(6)->push();
        lines.at(8)->push();
        while(difftime(std::time(nullptr), manager_time) != 10) { //because there is no stipulation of time to run, I set for one min
            auto time = std::time(nullptr); //this std::time(nullptr) gives current time
            std::cout << "Current time is " << time << std::endl;
            //**** ADJUST SPEED OF PUSH() NEW CUSTOMERS ON CHECKOUT LINE HERE by changing the modulo number ****
            auto n_customer = int(difftime(std::time(nullptr), manager_time)) % 3; // %2 for every 2 seconds.. 1 for every 1 second
            if(n_customer == 0)
            {
               //here we call get_next_line to sort and return the [0] element of the vector
               //then push a new customer to that line, whatever it happens to be after being sorted
               lines.at(get_next_line(lines))->push();
            }

            get_display(lines);
            checkout_shoppers(lines);
            std::this_thread::sleep_for(std::chrono::seconds(1)); //only non windows library for sleeping
            system("clear");  //clear screen
        }
        std::cout << "**** Closing: Finishing customer checkouts ****" << std::endl;
        closing_time(lines);
    }//end constructor
};//end checkout_manager class

void checkout_manager::checkout_shoppers(std::vector<checkout_line*> &shoppers)
{
    //because multi-threading was a bust
    // we call checkout on each line every second
    //there is logic built in on the node itself to compare its' timestamp with current time looking for 6 sec
    //before executing a pop().
    for (int i = 0; i < 9; i++)
    {
        shoppers.at(i)->checkout();
    }
}
void checkout_manager::get_display(std::vector<checkout_line *> &shoppers) const
{
    //Displays all lines (in the order that our get_next_line function) returns
    for (int i = 0; i < 9; i++)
    {
        std::cout << shoppers.at(i)->get_name() << "  ";
        shoppers.at(i)->display();
        std::cout << std::endl;
    }
}
int checkout_manager::get_next_line(std::vector<checkout_line *> &shoppers) const
{
    std::sort(shoppers.begin(), shoppers.end(),[](checkout_line *l1, checkout_line *l2){
        return l1->getLineCount() < l2->getLineCount();
    });
    //I use sort with a lambda function comparing 2 lines each iteration.
    //We make the assumption that line0 at the end of the sort will always be the smallest line.
    return shoppers.at(0)->getLineCount();
}
void checkout_manager::closing_time(std::vector<checkout_line*> &shoppers)
{
    int i=1;
    while(i > 0)
    {
        int temp =0;
        std::cout << "Closing.... remaining customer checkout only " << "\n";
        get_display(shoppers);
        checkout_shoppers(shoppers);
        for (int j = 0; j < 9; j++)
        {
            if(shoppers.at(j)->getLineCount() != 0)
            {
                temp++;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (temp == 0) { i = 0; } //exit cleanup
    }
}

int main() {
    checkout_manager manager;
    return 0;
}
