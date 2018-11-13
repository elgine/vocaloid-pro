#pragma once
#include <vector>
#include <stdint.h>
#include <memory>
using namespace std;
namespace vocaloid{
    template<typename T>
    class Buffer{
    protected:
        T *data_;
        uint64_t max_size_;
        uint64_t size_;
    public:
        explicit Buffer(){
            max_size_ = 0;
            size_ = 0;
			data_ = nullptr;
        }

        explicit Buffer(uint64_t max_size){
			max_size_ = 0;
			size_ = 0;
			data_ = nullptr;
            Alloc(max_size);
        }

        explicit Buffer(T* data, uint64_t len, uint64_t offset){
			max_size_ = 0;
			size_ = 0;
			data_ = nullptr;
            max_size_ = size_ = len;
            Alloc(len);
            Set(data, len, offset);
        }

        ~Buffer(){
            Dispose();
			delete[] data_;
			data_ = nullptr;
        }

        void Fill(T v){
            for(auto i = 0;i < size_;i++){
                data_[i] = v;
            }
        }

        void Fill(T v, uint64_t length, uint64_t offset){
            if(max_size_ < length){
                Alloc(length);
            }
            if(length > size_)size_ = length;
            for(auto i = 0;i < length;i++){
                data_[i + offset] = v;
            }
        }

        void Splice(uint64_t len, uint64_t offset = 0){
            for(auto i = offset;i < size_;i++){
                if(i + len < size_){
                    data_[i] = data_[i + len];
                }else{
                    data_[i] = 0.0f;
                }
            }
            size_ -= len;
        }

        void Add(T *data, uint64_t len, uint64_t offset = 0){
            uint64_t data_offset = size_;
            if(max_size_ < size_ + len){
                Alloc(size_ + len);
            }
            for(auto i = 0; i < len;i++){
                data_[i + data_offset] = data[i + offset];
            }
            size_ += len;
        }

        void Pop(T *out, uint64_t len, uint64_t offset = 0){
            auto last = min(offset + len, size_);
			memcpy(out, data_ + offset, last - offset);
            Splice(len, offset);
        }

        void Pop(Buffer<T> *buf, uint64_t len, uint64_t offset = 0){
            Pop(buf->Data(), len, offset);
        }

        void Set(const T* data, uint64_t len, uint64_t offset = 0, uint64_t dst = 0){
            Alloc(len);
            for(auto i = 0;i < len;i++){
                data_[i + dst] = data[i + offset];
            }
        }

        void Alloc(uint64_t size){
            if(max_size_ < size){
				T* new_data = nullptr;
				AllocArray(size, &new_data);
				if (data_ != nullptr) {
					memcpy(new_data, data_, max_size_);
					DeleteArray(&data_);
				}
				data_ = new_data;
                max_size_ = size;
            }
        }

        void SetSize(uint64_t size){
            if(size_ == size)return;
            if(size_ < size)Alloc(size);
            size_ = size;
        }

        void Dispose(){
            max_size_ = size_ = 0;
        }

        uint64_t Size(){
            return size_;
        }

        T* Data(){
            return data_;
        }

        uint64_t MaxSize(){
            return max_size_;
        }
    };
}