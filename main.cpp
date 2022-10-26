#include <iostream>
#include <vector>
#include <cassert>
#include <cinttypes>
#include <cstring>

using namespace std;

class Sand
{
    typedef uint_fast64_t type_grain;
#define SP_TYPE_GRAIN SCNuFAST64
    typedef uint_fast16_t type_sizes;
#define SP_TYPE_SIZES SCNuFAST16

    type_sizes height;   //y - length
    type_sizes width;    //x
    vector<vector<type_grain>> grains;

public:
    Sand(decltype(height) height, decltype(width) width) : height(height), width(width)
    {
        grains.assign(height, vector<uint_fast64_t>(width));
    }

    bool from_file(const char *name)
    {
        FILE* in = fopen(name, "r");
        if(in == nullptr)
            return false;

        type_sizes i, j;
        type_grain n;
        while(fscanf(in, "%" SP_TYPE_SIZES "\t%" SP_TYPE_SIZES "\t%" SP_TYPE_GRAIN "\n", &i, &j, &n) == 3)
        {
            grains[i][j] = n;
        }


        fclose(in);
        return true;
    }

    bool save_to_bmp(unsigned long long iter, const char *folder)
    {
        char full_name[1024];
        sprintf(full_name, "%s%llu.bmp", folder, iter);
        //printf("name: \"%s\"\n", full_name);
        FILE* out = fopen(full_name, "wb");

        if(!out)
            return false;

        unsigned char BMP_Header[] =
        {0x42, 0x4D,
         0x00, 0x00, 0x00, 0x00,
         0x00, 0x00,
         0x00, 0x00,
         0x36, 0x00, 0x00, 0x00};
        unsigned char DIB_Header[] =
        {0x28, 0x00, 0x00, 0x00,
         0x03, 0x00, 0x00, 0x00,    /// Width pix
         0x03, 0x00, 0x00, 0x00,    /// Height pix
         0x01, 0x00,
         0x18, 0x00,                ///24 bits for one pixel
         0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00,
         0x13, 0x0B, 0x00, 0x00,
         0x13, 0x0B, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00};

        constexpr unsigned char size_pixel = 3;
        constexpr unsigned char k_row = 4;

        memcpy(DIB_Header + 4, &width, 2);
        memcpy(DIB_Header + 8, &height, 2);
        //DIB_Header[14] = size_pixel * 8;

        size_t row_size = (size_pixel * width);
        size_t row_padding_size = (k_row - (row_size%k_row))%k_row;
        row_size += row_padding_size;
        size_t bitmap_size = row_size * height;

//        printf("row_padding_size: %zu\n", row_padding_size);
//        printf("row_size: %zu\n", row_size);
//        printf("bitmap_size: %zu\n", bitmap_size);

        auto *bitmap_data = new unsigned char[bitmap_size];

        enum Color {
            WHITE   = 0xFFFFFF,
            GREEN   = 0x408000,
            VIOLET  = 0x7608aa,
            YELLOW  = 0xffd800,
            BLACK   = 0x000000
        };

        {
            size_t i = 0;
            for(auto& row : grains)
            {
                for(auto grain : row)
                {
                    Color color;
                    switch (grain) {
                        case 0:
                            color = WHITE;
                            break;
                        case 1:
                            color = GREEN;
                            break;
                        case 2:
                            color = VIOLET;
                            break;
                        case 3:
                            color = YELLOW;
                            break;
                        default:
                            color = YELLOW;
                            break;
                    }
                    memcpy(bitmap_data + i, &color, size_pixel);
                    i += size_pixel;
                }
                int padding = 0;
                memcpy(bitmap_data + i, &padding, row_padding_size);
                i += row_padding_size;
            }
        }

        for(auto c : BMP_Header)
            fprintf(out, "%c", c);
        for(auto c : DIB_Header)
            fprintf(out, "%c", c);
        for(size_t i = 0; i < bitmap_size; i++)
        {
            fprintf(out, "%c", bitmap_data[i]);
        }

        fclose(out);
        return true;
    }

    bool step()
    {
        bool shift_height = false, shift_width = false;

        for(size_t j = 0; j < grains[0].size(); j++)
            if(grains[0][j] >= 4)
            {
                shift_height = true;
                break;
            }
        for(size_t j = 0; j < grains[0].size(); j++)
            if(grains[0][j] >= 4)
            {
                shift_width = true;
                break;
            }

        Sand new_step(height + shift_height, width + shift_width);
        size_t n = 0;

        for(size_t i = 0; i < grains.size(); i++)
        {
            for(size_t j = 0; j < grains[i].size(); j++)
            {
                if(grains[i][j] >= 4)
                {
                    new_step.get(shift_height + i-1,    shift_width + j)    += 1;
                    new_step.get(shift_height + i,      shift_width + j-1)  += 1;
                    new_step.get(shift_height + i+1,    shift_width + j)    += 1;
                    new_step.get(shift_height + i,      shift_width + j+1)  += 1;
                    new_step.get(shift_height + i,      shift_width + j)    += get(i, j) - 4;
                    n++;
                }
                else
                {
                    new_step.get(shift_height + i, shift_width + j) += get(i, j);
                }
            }
        }

        if(n)
            swap(new_step);

        return n == 0;
    }


private:
    void swap(Sand &other)
    {
        std::swap(height, other.height);
        std::swap(width, other.width);
        std::swap(grains, other.grains);
    }

    type_sizes get_height()
    {
        return height;
    }

    type_sizes get_width()
    {
        return width;
    }

    void add_line_top()
    {
        grains.insert(grains.begin(), vector<type_grain>());
    }

    void add_line_left()
    {
        for(auto &v : grains)
        {
            v.insert(v.begin(), type_grain{0});
        }
    }


    void resize(type_sizes y, type_sizes x)
    {
        assert(y <= height);
        if(y == height)
        {
            height = y + 1;
            grains.resize(height, vector<type_grain>(width)); ///!!!!!!!!!!!!!!!!!!!!!!!!!!!
        }
        assert(x <= grains[y].size());
        if(x == grains[y].size())
        {
            width = x + 1;
            grains[y].push_back(0);
        }
    }

    void set(type_sizes y, type_sizes x, type_grain val)
    {
        resize(y, x);
        grains[y][x] = val;
    }

    type_grain& get(type_sizes y, type_sizes x)
    {
        resize(y, x);
        return grains[y][x];
    }

    friend std::ostream& operator<<(std::ostream& stream, const Sand& sand);
};

std::ostream& operator<<(std::ostream& stream, const Sand& sand) {

    for(auto &row : sand.grains)
    {
        size_t i = 0;
        for(auto &el : row)
        {
            stream << el << " ";
            i++;
        }
        while(i < sand.width)
        {
            stream << "0 ";
            i++;
        }
        stream << endl;
    }
    return stream;
}

int main() {
    Sand sand(333, 333);
    sand.from_file("in.tsv");
    //cout << sand << endl;
    sand.save_to_bmp(0, "C:\\Sand\\");
    for(unsigned long long i = 1; i < 1000000 && !sand.step(); i++){
        if(i%1000 == 0)
        {
            cout << i << endl;
            sand.save_to_bmp(i, "C:\\Sand\\");
        }

    }


    return 0;
}



//    vector<int> a;
//    a.assign(5, 0);
//    for(auto v : a)
//    {
//        cout << v << " ";
//    }
//    cout << endl;
//    a.resize(10, 0);
//    for(auto v : a)
//    {
//        cout << v << " ";
//    }
//    cout << endl;

//
//    vector<vector<uint_fast64_t>> sand;
//
//    sand.assign(height, vector<uint_fast64_t>(width));
//
//    for(auto &row : sand)
//    {
//        for(auto &el : row)
//        {
//            el = 8;
//        }
//    }
//
//    for(auto &row : sand)
//    {
//        row[10] = 0;
//    }
//
//
//    for(auto &row : sand)
//    {
//        for(auto &el : row)
//        {
//            cout << el << " ";
//        }
//        cout << endl;
//    }
//
//    std::vector<int> c = {1, 2, 3};
//    std::cout << "The vector holds: ";
//    for (const auto& el: c) std::cout << el << ' ';
//    std::cout << '\n';
//    c.resize(5);
//    std::cout << "After resize up to 5: ";
//    for (const auto& el: c) std::cout << el << ' ';
//    std::cout << '\n';
