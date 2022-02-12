#pragma once 

#include <unordered_map>
#include <functional>
#include <deque>


namespace utils
{  
    //find unordered_map
    template<typename M, typename G,typename T>
    T* FindUnorderedMap(const M& name, std::unordered_map<G, T>& data)
    {
        //search for the object, and return nullpointer if not found
        auto it = data.find(name);
        if (it == data.end()) {
            return nullptr;
        }
        else {
            return &it->second;
        }
    }


    struct FunctionQueuer
    {
        std::deque<std::function<void()>> executer;

        void PushFunction(std::function<void()>&& function) {
            executer.push_back(function);
        }


        /**
        *@param inverse if true, functions will be called in order.
        */
        void Flush(const bool& inverse = false) {
            if(inverse == true)
            {
                for (auto it = executer.begin(); it != executer.end(); it++) {
                    (*it)(); //call functors
                } 
            }
            else {
                // reverse iterate the executer queue to execute all the functions
            for (auto it = executer.rbegin(); it != executer.rend(); it++) {
                    (*it)(); //call functors
                } 
            }
            executer.clear();
        }
    };
}