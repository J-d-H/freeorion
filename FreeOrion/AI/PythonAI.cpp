#include "PythonAI.h"
#include "../util/AppInterface.h"

#include <boost/python.hpp>

////////////////////////
// Python AIInterface //
////////////////////////
BOOST_PYTHON_MODULE(foaiint)
{
    boost::python::def("EmpirePlayerID", AIInterface::EmpirePlayerID);
    boost::python::def("CurrentTurn", AIInterface::CurrentTurn);

    boost::python::def("DoneTurn", AIInterface::DoneTurn);

    boost::python::def("LogOutput", AIInterface::LogOutput);
}
 
///////////////////////
//     PythonAI      //
///////////////////////
using boost::python::borrowed;

PythonAI::PythonAI()
{
    Py_Initialize();    // initializes Python interpreter, allowing Python functions to be called from C++

    initfoaiint();   // allows the "foaiint" C++ module to be imported within Python code

    try {
        main_module = PyOBJECT((PyHANDLE(borrowed(PyImport_AddModule("__main__")))));
        dict = PyOBJECT(main_module.attr("__dict__"));
    } catch (PyERROR err) {
        Logger().errorStream() << "error with main module or namespace";
        return;
    }

    PyHANDLE handle;

    try {
        handle = PyHANDLE(PyRun_String("import sys", Py_file_input, dict.ptr(), dict.ptr()));
    } catch (PyERROR err) {
        Logger().errorStream() << "error importing sys";
        return;
    }

    
    std::string AI_DIR("C:\\FreeOrion\\AI");
    std::string python_path_command = "sys.path.append('" + AI_DIR + "')";
    try {
        handle = PyHANDLE(PyRun_String(python_path_command.c_str(), Py_file_input, dict.ptr(), dict.ptr()));
    } catch (PyERROR err) {
        Logger().errorStream() << "error appending to sys.path";
        return;
    }


    try {
        handle = PyHANDLE(PyRun_String("import FreeOrionAI", Py_file_input, dict.ptr(), dict.ptr()));
    } catch (PyERROR err) {
        Logger().errorStream() << "error importing blah";
        return;
    }


    try {
        handle = PyHANDLE(PyRun_String("FreeOrionAI.InitFreeOrionAI()", Py_file_input, dict.ptr(), dict.ptr()));
    } catch(PyERROR err) {
        Logger().errorStream() << "error calling FreeOrionAI.InitFreeOrionAI()";
        return;
    }

    Logger().debugStream() << "Initialized PythonAI";
}

PythonAI::~PythonAI()
{
    std::cout << "Cleaning up / destructing Python AI" << std::endl;
    Py_Finalize();      // stops Python interpreter and release its resources
}

void PythonAI::GenerateOrders()
{
    try {
        PyHANDLE handle = PyHANDLE(PyRun_String("FreeOrionAI.GenerateOrders()", Py_file_input, dict.ptr(), dict.ptr()));
    } catch(PyERROR err) {
        Logger().errorStream() << "error calling FreeOrionAI.GenerateOrders()";
        AIInterface::DoneTurn();
        return;
    }
}

void PythonAI::HandleChatMessage(int sender_id, const std::string& msg)
{}
