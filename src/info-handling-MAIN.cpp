#include <cstddef>
#include<iostream>
#include <stdexcept>
#include <string>
//#include <typeinfo>
#include<vector>

class data_struct {
  private:
    int nodeID;
    int pluginID;

    bool added_records;

    // equivalent to Opis_DATA from USTALENIA.md
    class data_description {
      private:
        std::string type; // here we will pass to the constructor what type we want to store

        std::string data_name; // just for json purposes

      public:
        data_description () {
          type = "NO TYPE ASSIGNED";;
          data_name = "NOT ASSIGNED";
        }

        data_description(std::string t, std::string n) {
          type = t;
          data_name = n;
        }

    };

    int description_list_length;

    std::vector<data_description> list_data_description; // this raskall is going to store the information about the data about what data we want to e.g. send with this object
 

    int stored_list_length;
    std::vector< std::pair < int, std::vector<void *> > > stored_data; // list of values stored as void *
                                                                       // we will be able to reenginere their types thanks to information stored in list_data_description
                                                                       // that is, vector that is the second element of each of those pairs will have the same arrangement as list_data_description


  public:
    // just a default constructor cuz why wouldnt you want 666 to appear in your code?
    data_struct() {
      added_records = false;
      description_list_length = 0;

      nodeID = -666;
      pluginID = -666;
    }

    // i don't think that passing lists to the constructor is a good ideal
    // i would rather have methods to add elements to list_data_description and then a seperate method to pass stored data
    data_struct(int nID, int pID) {
      added_records = false;
      description_list_length = 0;
      stored_list_length = 0;
  
      nodeID = nID;
      pluginID = pID;
    }

    void push_data_description (std::string type_name, std::string d_name) {
      if (added_records)
        throw std::invalid_argument("You cannot alter data_description after saving records");

      // std::string type_name = typeid(data).name(); <- might be useful someday :v

      data_description * new_description = new data_description (type_name, d_name);
      
      list_data_description.push_back(*new_description);

      description_list_length++;
    }

    // here i want to have 3 versions of the same method: one that accepts only new record, one that accepts a new record and time and one that accepts time and list of records

    void push_data(void * data) {
      if ( stored_list_length < 1 || description_list_length <= std::size(stored_data[stored_list_length - 1].second) ) // if there are no records saved yet or record list for previous time is full then be fussy about it
        throw std::invalid_argument("You must specify the time of new measurement");

      stored_data[stored_list_length -1].second.push_back(data);
    }

    void push_data(int t, void * data) {
      if ( description_list_length > std::size(stored_data[stored_list_length - 1].second) )
        throw std::invalid_argument("You cannot add new measurement if the previous one is not finished");

      std::vector<void *> temp_vect (1, data);
      stored_data.push_back( make_pair(t, temp_vect) );

      stored_list_length++;
    }

    void push_data(int t, std::vector<void *>data) {
      if ( description_list_length > std::size(stored_data[stored_list_length - 1].second) )
        throw std::invalid_argument("You cannot add new measurement if the previous one is not finished");

      stored_data.push_back( make_pair(t, data) );

    }
};
