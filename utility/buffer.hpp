#pragma once
#include <stdint.h>
#include <memory>
using namespace std;
namespace vocaloid{
    template<typename T>
    class Buffer{
    protected:
        T *data_;
        int64_t max_size_;
        int64_t size_;
    public:
        explicit Buffer(){
            max_size_ = 0;
            size_ = 0;
			data_ = nullptr;
        }

        explicit Buffer(int64_t max_size){
			max_size_ = 0;
			size_ = 0;
			data_ = nullptr;
            Alloc(max_size);
        }

        ~Buffer(){
            Dispose();
			DeleteArray(&data_);
        }

        void Fill(T v, int64_t length, int64_t offset){
            if(max_size_ < length){
                Alloc(length);
            }
            if(length > size_)size_ = length;
			if (v == 0) {
				memset(data_ + offset, v, length * sizeof(T));
			}
			else {
				for (auto i = 0; i < length; i++) {
					data_[i + offset] = v;
				}
			}	
        }

        void Splice(int64_t len, int64_t offset = 0){
			len = min(offset + len, max_size_) - offset;
			if (len <= 0)return;
			int64_t end_index = offset + len;
			int64_t count = max_size_ - offset - len;
			memcpy(data_ + offset, data_ + end_index, count * sizeof(T));
			memset(data_ + end_index, 0, count * sizeof(T));
			size_ -= len;
        }

        void Add(T *data, int64_t len, int64_t offset = 0){
            int64_t data_offset = size_;
            if(max_size_ < size_ + len){
                Alloc(size_ + len);
            }
			/*for (auto i = 0; i < len; i++) {
				data_[i + data_offset] = data[i + offset];
			}*/
			memcpy(data_ + data_offset, data + offset, len * sizeof(T));
            size_ += len;
        }

        int64_t Pop(T *out, int64_t len, int64_t offset = 0){
            len = min(offset + len, size_) - offset;
			/*for (auto i = 0; i < len; i++) {
				out[i] = data_[i + offset];
			}*/
			memcpy(out, data_ + offset, len * sizeof(T));
            Splice(len, offset);
			return len;
        }

        void Pop(Buffer<T> *buf, int64_t len, int64_t offset = 0){
            Pop(buf->Data(), len, offset);
        }

        void Set(const T* data, int64_t len, int64_t offset = 0, int64_t dst = 0){
            Alloc(dst + len);
			/*for (auto i = 0; i < len; i++) {
				data_[i + dst] = data[i + offset];
			}*/
			memcpy(data_ + dst, data + offset, len * sizeof(T));
        }

        void Alloc(int64_t size){
            if(max_size_ < size){
				T* new_data = nullptr;
				AllocArray(size, &new_data);
				if (data_ != nullptr) {
					memcpy(new_data, data_, max_size_ * sizeof(T));
					DeleteArray(&data_);
				}
				data_ = new_data;
                max_size_ = size;
            }
        }

        void SetSize(int64_t size){
            if(size_ == size)return;
            if(size_ < size)Alloc(size);
            size_ = size;
        }

        void Clear(){
            size_ = 0;
        }

        void Dispose(){
            max_size_ = size_ = 0;
            DeleteArray(&data_);
        }

        int64_t Size(){
            return size_;
        }

        T* Data(){
            return data_;
        }

        int64_t MaxSize(){
            return max_size_;
        }
    };
}