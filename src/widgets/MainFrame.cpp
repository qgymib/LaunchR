#include "MainFrame.hpp"

enum
{
    ID_Hello = 1
};

LR::MainFrame::MainFrame(wxWindow* parent) : wxFrame(nullptr, wxID_ANY, "Hello World")
{
    (void)parent;
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(ID_Hello, "&Hello...\tCtrl-H", "Help string shown in status bar for this menu item");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");

    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("Welcome to wxWidgets!");

    Bind(wxEVT_MENU, &MainFrame::OnHello, this, ID_Hello);
    Bind(wxEVT_MENU, &MainFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MainFrame::OnExit, this, wxID_EXIT);
}

LR::MainFrame::~MainFrame()
{
}

void LR::MainFrame::OnExit(wxCommandEvent&)
{
    Close(true);
}

void LR::MainFrame::OnAbout(wxCommandEvent&)
{
    wxMessageBox("This is a wxWidgets Hello World example", "About Hello World", wxOK | wxICON_INFORMATION);
}

void LR::MainFrame::OnHello(wxCommandEvent&)
{
    wxLogMessage("Hello world from wxWidgets!");
}