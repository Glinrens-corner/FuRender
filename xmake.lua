option("modus")
    set_showmenu(true)
    set_values("full","lib")
    set_default("full")
option_end()




add_cxxflags("-march=native", {tools = {"clang", "gcc"}})
add_cxxflags("-Wno-c++98-compat", {tools = {"clang"}, force=true})

add_rules("mode.debug")

if is_mode("debug") then
   add_cxxflags("-fno-omit-frame-pointer", {tools = {"clang", "gcc"}})
   add_cxxflags("-ggdb3", {tools = {"clang", "gcc"},force=true})
   set_symbols("debug")
   set_optimize("none")
end

target("basis_framework_glinren_furender")
    set_kind("static")
    set_default(true)
    set_basename("furender")
    add_files("src/*.cpp")
    set_languages("cxx17")
    add_headerfiles("include/*.hpp","src/*.hpp")
    add_includedirs("include","src")
target_end()

if is_config("modus", "full") then
add_requires("vcpkg::catch2 3.6.0")

    
target("test")
    set_kind("binary")
    set_filename("furender_test")
    set_targetdir(".")
    add_files("src/*.cpp")
    add_packages("vcpkg::catch2")
    set_warnings("all")
--    set_optimize("fastest")
    set_languages("cxx17")
    add_includedirs("include","src")
--    add_links("Catch2Main", "Catch2" )
    add_files("test/*.cpp")

end
