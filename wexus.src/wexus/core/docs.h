
/**
  \page wexuscorepage Wexus Core Reference
  
   - \subpage wexusinstallingpage
   - \subpage wexushelloworldpage
   - \subpage wexusoverviewpage
   - \subpage wexusturbopage
   - \subpage wexuswxmakepage

*/

/**
  \page wexusinstallingpage Compiling and Installing
  
    - Download the source from the file area: http://labs.wexussoftware.com/files/
    - Extract the source with tar
    - Make/choose an install directory - this guide assumes you'll want to install the library to ~/lib
    - Copy bin/scopiraflags.pc to your ~/lib
    - Add ~/lib to your PKG_CONFIG_PATH and LD_LIBRARY_PATH:

  \verbatim
  export PKG_CONFIG_PATH=$HOME/lib:$PKG_CONFIG_PATH
  export LD_LIBRARY_PATH=$HOME/lib:$LD_LIBRARY_PATH
  \endverbatim

    - From within the extracted directory, compile and build wexus:

  \verbatim
  bin/buildboss scopira.src wexus.src -O ~/lib
  make
  \endverbatim

    - You should now have a few .so files and ''wxmake'' in your ''~/lib''. You may want to move wxmake to somewhere in your ''PATH''
    - You may also want to move the ''buildboss'' utility from bin to somewhere in your ''PATH''

*/

/**
 \page wexushelloworldpage Quick tutorial

 This tutorial will show you how to build a basic ''Hello world'' Wexus
 application.
 It assumes you've followed the instructions in \ref wexusinstallingpage

 \section dirsec Setting up directories

 Create a directory for you ''Hello, World'' first-application:

 \verbatim
 mkdir hello.src
 cd hello.src
 \endverbatim

 \section basicsec Basic controller

 Using the ''wxmake'' utility, lets create the core .cpp file for the application:

 \verbatim
 wxmake mkapp=hello
 \endverbatim

 This will create hello.h and .cpp.

 This is the core .cpp file for your application. It contains the ''main()'' function, startup
 methods and other configuration parameters. By default, this contains generic ODBC database
 initialization code -- which we will disable for this tutorial. Open the hello.cpp file and comment
 out the following two lines like so:

 \verbatim
 //b = m_dbpoolmgr.init_pool(2, "ademko", "ademko", "");
 //assert(b && "[db failed to open]");
 \endverbatim

 Next, we will make a controller object. Everytime a user loads a page, a controller object
 is instatiated, one of its methods is called, and then the controller instance is destroyed.
 Controller objects therefore collect similar methods or page handlers into concrete objects.

 Lets make our first controller, the index page:

 \verbatim
 wxmake mkcontroller=main
 \endverbatim

 This will create main_co.h and .cpp (_co for controller), with a basic shell of a controller.

 At this point, you have a fully functioning app with a single controller (which has one user-callable
 method, index()). You can proceed to \ref compilesec to compile and run the application now, or proceed
 to the next section to add a ''wx'' file to the project.

 \section wxfilesec Adding a .wx file

 If you look inside these files, you will find the definition of the index() method.
 This method has a redumentary body filled out for you, which is functional. It is also registered
 as a user-callable method, that is, loadable via the web page, by the register_controller line.

 We will now replace the implementation of the index() method. Rather than render one line of text to the
 screen using EVENT.output(), we will call an internall method ''render()''. This method will be implemented
 as a .wx file.

 Add the following method signature
 to the .h file (right under the existing index() signature):

 \verbatim
 private:
   void render(void);
 \endverbatim

 And replace the EVENT... line in the body of index() with a call to render():

 \verbatim
 render();
 \endverbatim

 However, rather than implementing the body of main::render() in main_co.cpp, we will
 make a .wx file. To create the initial .wx file, do:

 \verbatim
 wxmake mkcontroller=main mkmethod=render
 \endverbatim

 This will create main_co.render.wx. wxmake will not overwrite your main_co.h and .cpp files --
 wxmake never overwrites existing files.

 You may tweak the presentation HTML code in the .wx file before compiling, if you'd like.
 Run wxmake without any parameters to see the various tag types.

 \section compilesec Compiling the project

 The quickest method to compile a Wexus based project is to use wxmake to make a custom
 Makefile:

 \verbatim
 wxmake mkmakefile=. makefile=Makefile
 \endverbatim

 You may then use ''make'' to compile your application:

 \verbatim
 make
 \endverbatim

 This should compile and link your application. You need only rerun make when you make changes to your source
 files. However, if you add or remove .cpp (even with wxmake) files from your project,
 you must remake the makefile with the ''wxmake mkmakefile...'' command.

 If you created the .wx file portion of this tutorial, you can look at the intermediate code
 in main_co.render.cpp

 \section runsec Running the application

 Finally, run your application with:

 \verbatim
 ./wxapp
 \endverbatim

 You can then access http://localhost:8080/hello/main_co.index.wx

*/

/**
  \page wexusoverviewpage Overview

  \section whysec Why Wexus?

  Wexus is a C++ library for building web applications. It is not designed to compete with interpretted languages and systems such as Java or Ruby, rather it has the following goals:

    - Programmers proficient in C++ that would like to use C++'s advantages of compiled-time speed and static type safety
    - A usable librarie for C++ programmers who have existing code they'd like to web enable
    - Web sites that require maximum performance, scalability and flexibility

  \section stackedsec Stacked Design

  Wexus utilizes a flexible, stacked design to building applications. The layers include:

    - A tool layer provides serialization, I/O, threading, smart pointers (reference counting)
    - The core layer provides the core engine dispatcher
    - You attach one or more fronts to the engine. Usually, this is atleast one http (web server) front.
    - On the back end of the engine, you attach one more more reactors. On every event (ie. web page load), the engine proceeds down the reactor stack until one of the reactors claims and handles the event

  The flexibility lies in using and combining various reactors. These include:

    - A collection of utily reactors, including URL remapping and redirection
    - Static file file serving, including directory index production
    - A reactor that supports Wexus Record application development. This style is considered deprecated, but is still useful and used at http://lendstuff.wexussoftware.com/
    - A reactor that supports Wexus Turbo application development (see next secion)
    - Any other reactors or user-made reactors

  \section usingapachesec Using Apache

  The HTTP front included with wexus is multi-threaded and robust and can be used to serve your web site directly. However, one would usually place a wexus-based application behind an apache server, and tell the apache server to forward all the urls that end in a specific extention (like .wx) to the internal wexus server. This can be done with two lines of apache .conf:

   RewriteEngine On
  RewriteRule ^/(.*\.wx)$ http://127.0.0.1:2000/$1 [P,L]

  (where 2000 is the port that the wexus-based application's web server is listening on)

  \section usingturbosec Wexus Turbo

  Most application development in wexus would be done with the Turbo extension/reactor. Turbo provides the following web application paradigms:

    - PHP-like (mixed C++ and HTML in the same file)
    - Controller/View style (ala Ruby on Rails)
    - An ORM layer (ie Data layer) is currently under consideration

  You can mix all these styles in the same application, ofcourse, aswell as with other reactors (paradigms).See Wexus:Turbo:overview for more information on the Turbo extension.

  \section devprocsec Development Process

  Wexus is correctly being used for various internal and external projects. However, external users are greatly encouraged to use the library, give feed back and contribute addtions. Only through a open, community based development library can the library be grown to support many domains and coding requirements.  

*/

/**
  \page wexusturbopage Turbo Overview

  The Wexus Turbo extension provides a the ability to mix C++ code and HTML code in the same file, and render the results. This facility is provided by the classes and utilities in the wexus::turbo namespace, as well as the wxmake utility program.

  \section wxmakesec wxmake

  For a more detailed introduction to wxmake, see \ref wexuswxmakepage. wxmake is a utility program that has the following features:

  \subsection renderingsec Rendering .wx files

  A .wx file is a file that contains both C++ and HTML code. wxmake can take such a file and produce a pure .cpp source file that can then be compiled by the C++ compiler.

  \subsection prodmakefilessec Producing Makefiles

  Given a source directory filed with .wx and .cpp files, wxmake can produce a makefile for use with the standard make utility. In particular, it handles the proper rules to make sure .wx files produce .cpp files, which produce .o files and are finally linked to the final executable.

  \subsection codegensec Code generation

  Finally, wxmake can be used to generate:
    - Your application shell/main program
    - Controller objects
    - Methods for controllers

  \section controllerviewsec Controller/View

  Turbo supports an object-oriented style of the Controller/View paradigm. In this system, when the user accesses a page, the turbo reactor will core dispatch this to the proper controller. This invovles:

    - Finding the given turbohandler for the requested url.
    - C/V handlers will then instantiate an instance of the controlelr class
    - The contructor of the class didn't throw any errors or exceptions, the associated method is then called on the instance to handle the request
    - The associated method performance any logic, aswell as sets up any member variables
    - The method will then call its own internal render method
    - The render method will then render the page, using the setup member variables
    - Typically, render methods are developed using .wx files (ie. mixed C++ and HTML code). Other paradigms are also possible (for example, a RAII-based XML production facility)

  \section wxfilessec .wx files

  .wx files are simply mixed C++ & HTML files that are compiled into .cpp files, and then linked with the application.

  They have two primary uses:

    - Providing a way to implement the various render methods in a controller class (as part of the Controller/View paradigm)
    - Making stand along pages (PHP-like). In this case, the page will simply handle the full request directly.

  \section eventsyssec EVENT system

  Handlers, render pages and stand alone .wx pages all interact with the web user through the EVENT object. This is a global(1) object of type turboevent. This object provides:

    - Read (and "write") access to the user's form fields
    - An error facility, where you can append errors, inspect them, etc
    - Access to a database handle, that will automatically be taken from the connection pool
    - Access to the output stream, including an HTML-encoding version

  (1) it's not actually a global, but stored in thread-local-storage (TLS), allowing for multiple threads to each have their own EVENT objects.

  \section utilsec Utilities

  Finally, there is a slew of utility functions and classes that build on the EVENT system. These provide:

    - Form building and rendering
    - Form field data extraction and validation
    - Other validators, extractors and formatters
    - A Turbo-enhanced DB statement object for easy query building and binding
    - A exception safe RAII-type DB transaction object
    - A standard usersession object that can be built up. This includes standard user-tracking items such as userid, name, password, aswell as a intercall-like Ruby-flash contruct (named notes).
    - Password hashing functions

*/

/**
  \page wexuswxmakepage wxmake Utility

   \section quickhelpsec Quick Help

  Type wxmake without any options for a quick help screen. A more detailed page will be provided here, later.

*/

