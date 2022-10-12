#include <cstdlib>
#include <cxxopts.hpp>

#include <window.hpp>

int32_t main(int32_t argc, char** argv)
{
    cxxopts::Options options = cxxopts::Options{ "Visualizer", "OpenGL window allowing to visualize the students' answers" };

    options.add_options()
        ("d,debug", "Enables OpenGL debugging mode", cxxopts::value<bool>()->default_value("false"))
        ("h,help", "Print usage")
        ;

    cxxopts::ParseResult commandLineOptions = options.parse(argc, argv);

    if (commandLineOptions.count("help"))
    {
        std::cout << options.help() << std::endl;
        return EXIT_SUCCESS;
    }

    auto &window = visualizer::Window::GetInstance();

    if (!window.InitWindow("OpenGL forest - 3D Programming Course", 1280, 720, commandLineOptions))
    {
        return EXIT_FAILURE;
    }

    window.Run();

    return EXIT_SUCCESS;
}
