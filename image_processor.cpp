#include "errors.h"
#include "image.h"
#include "bmp_in.h"
#include "bmp_out.h"
#include "factory.h"
#include "parse_args.h"
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    const std::vector<std::string_view> raw_args(argv + 1, argv + argc);
    try {
        const Args args = PargeArgs(raw_args);
        std::vector<std::unique_ptr<Filter>> filters;
        for (const auto& type : args.filters) {
            filters.push_back(CreateFilter(type));
        }
        Image image = ImageRead::Read(args.input);
        for (const auto& flt : filters) {
            flt->Apply(image);
        }
        ImageWrite::Write(image, args.output);
    } catch (const ERROR::BMPError& Error) {
        std::cerr << "BMPError: " << Error.what() << '\n';
        return 1;
    } catch (const ERROR::FileError& Error) {
        std::cerr << "FileError: " << Error.what() << '\n';
        return 1;
    } catch (const ERROR::IncorrectFilter& Error) {
        std::cerr << "IncorrectFilter: " << Error.what() << '\n';
        return 1;
    } catch (const ERROR::OutOfRange& Error) {
        std::cerr << "OutOfRange: " << Error.what() << '\n';
        return 1;
    } catch (const ERROR::InvalidArgument& Error) {
        std::cerr << "InvalidArgument: " << Error.what() << '\n';
        return 1;
    } catch (const ERROR::Exception& Error) {
        std::cerr << Error.what() << '\n';
        return 1;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
