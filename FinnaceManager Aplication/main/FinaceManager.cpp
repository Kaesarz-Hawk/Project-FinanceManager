// pro_finance_manager_final_v6.cpp
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <map>
#include <set>
#include <cctype>

using namespace std;


class FinanceApp;
void set_theme(); 
void restyle_group(Fl_Group* group);
void restyle_window_recursively(Fl_Widget* w);


Fl_Color CLR_BG, CLR_CARD, CLR_INPUT_BG, CLR_PRIMARY, CLR_SUCCESS, CLR_DANGER, CLR_TEXT, CLR_SUBTEXT;

// Theme
const Fl_Color CLR_DARK_BG       = fl_rgb_color(30, 33, 40);
const Fl_Color CLR_DARK_CARD     = fl_rgb_color(45, 48, 58);
const Fl_Color CLR_DARK_INPUT_BG = fl_rgb_color(60, 63, 75);
const Fl_Color CLR_DARK_PRIMARY  = fl_rgb_color(64, 158, 255);
const Fl_Color CLR_DARK_SUCCESS  = fl_rgb_color(100, 210, 140);
const Fl_Color CLR_DARK_DANGER   = fl_rgb_color(255, 100, 100);
const Fl_Color CLR_DARK_TEXT     = fl_rgb_color(240, 240, 245);
const Fl_Color CLR_DARK_SUBTEXT  = fl_rgb_color(160, 165, 180);

// UTILS
namespace Utils {
    string getCurrentDate() {
        time_t now = time(0);
        tm *ltm = localtime(&now);
        char buffer[20];
        strftime(buffer, 20, "%Y-%m-%d", ltm);
        return string(buffer);
    }
    string generateID() {
        return to_string(time(0)) + "-" + to_string(rand() % 1000000);
    }
    string sanitizeFilename(const string &s) {
        string out;
        for(char c : s) {
            if(isalnum((unsigned char)c) || c=='-' || c=='_') out.push_back(c);
            else out.push_back('_');
        }
        if(out.empty()) out = "user";
        return out;
    }
}

// DATA MODELS
class Transaction {
    string id, type, category, note, date;
    double amount;
public:
    Transaction() : amount(0.0) { id = Utils::generateID(); }
    Transaction(string t, double a, string c, string n, string d, string existingId = "")
        : type(t), amount(a), category(c), note(n), date(d) {
        id = existingId.empty() ? Utils::generateID() : existingId;
    }
    string getId() const { return id; }
    string getType() const { return type; }
    double getAmount() const { return amount; }
    string getCategory() const { return category; }
    string getNote() const { return note; }
    string getDate() const { return date; }
    void setType(const string &t) { type = t; }
    void setAmount(double a) { amount = a; }
    void setCategory(const string &c) { category = c; }
    void setNote(const string &n) { note = n; }
    void setDate(const string &d) { date = d; }
};

struct Budget { string category; double limit; double spent; };
struct ReportStats { double income; double expense; double savings; };

// CUSTOM WIDGETS
class CardGroup : public Fl_Group {
public:
    CardGroup(int x, int y, int w, int h, const char* l = 0) : Fl_Group(x,y,w,h,l) {
        box(FL_RFLAT_BOX);
        color(CLR_CARD);
        if(l) labelcolor(CLR_TEXT);
    }
};

class PieChart : public Fl_Widget {
    double income, expense;
public:
    PieChart(int x,int y,int w,int h) : Fl_Widget(x,y,w,h), income(0), expense(0) {}
    void updateValues(double inc, double exp) { income=inc; expense=exp; redraw(); }
    void draw() override {
        fl_color(CLR_CARD); fl_rectf(x(),y(),w(),h());

        if(income==0 && expense==0){
            fl_color(CLR_SUBTEXT);
            fl_font(FL_HELVETICA, 14);
            fl_draw("No Data", x()+w()/2-25, y()+h()/2); return;
        }
        double total=income+expense;
        double incAngle = (income/total)*360;
        int cx = x()+w()/2, cy=y()+h()/2, r=(w()<h()?w():h())/2-15;

        if(income>0){ fl_color(CLR_SUCCESS); fl_pie(cx-r,cy-r,2*r,2*r,0,incAngle); }
        if(expense>0){ fl_color(CLR_DANGER); fl_pie(cx-r,cy-r,2*r,2*r,incAngle,360); }

        fl_color(CLR_CARD); fl_pie(cx-r/2, cy-r/2, r, r, 0, 360);
    }
};

// THEME STYLING HELPERS
void set_theme() {
    CLR_BG = CLR_DARK_BG; CLR_CARD = CLR_DARK_CARD; CLR_INPUT_BG = CLR_DARK_INPUT_BG;
    CLR_PRIMARY = CLR_DARK_PRIMARY; CLR_SUCCESS = CLR_DARK_SUCCESS; CLR_DANGER = CLR_DARK_DANGER;
    CLR_TEXT = CLR_DARK_TEXT; CLR_SUBTEXT = CLR_DARK_SUBTEXT;
}

void styleButton(Fl_Button* b, Fl_Color c) {
    if (!b) return;
    b->color(c);
    b->labelcolor(FL_WHITE);
    b->box(FL_RFLAT_BOX);
    b->labelfont(FL_HELVETICA_BOLD);
    b->clear_visible_focus();
}

void styleInput(Fl_Input* i) {
    if (!i) return;
    i->box(FL_FLAT_BOX);
    i->color(CLR_INPUT_BG);
    i->textcolor(CLR_TEXT);
    i->selection_color(CLR_PRIMARY);
    i->textfont(FL_HELVETICA);
    i->labelfont(FL_HELVETICA);
    i->labelcolor(CLR_SUBTEXT);
}


void restyle_group(Fl_Group* group) {
    if (!group) return;

    group->color(CLR_BG);
    group->redraw();

    for (int i = 0; i < group->children(); ++i) {
        Fl_Widget* w = group->child(i);
        if (!w) continue;

        if (dynamic_cast<Fl_Double_Window*>(w) || dynamic_cast<Fl_Group*>(w)) {
            w->color(CLR_BG);
            if (Fl_Tabs* tabs = dynamic_cast<Fl_Tabs*>(w)) {
                tabs->color(fl_rgb_color(35, 38, 45));
                tabs->selection_color(CLR_PRIMARY);
                tabs->labelcolor(CLR_SUBTEXT);
                tabs->box(FL_FLAT_BOX);
            }
            if (dynamic_cast<CardGroup*>(w)) {
                w->color(CLR_CARD);
            }
            if (Fl_Group* g = dynamic_cast<Fl_Group*>(w)) restyle_group(g);
        }
        // Inputs
        else if (Fl_Float_Input* fi = dynamic_cast<Fl_Float_Input*>(w)) {
            styleInput(fi);
        }
        else if (Fl_Input* in = dynamic_cast<Fl_Input*>(w)) {
            styleInput(in);
        }
        else if (Fl_Choice* c = dynamic_cast<Fl_Choice*>(w)) {
            c->box(FL_FLAT_BOX); c->color(CLR_INPUT_BG); c->textcolor(CLR_TEXT); c->labelcolor(CLR_SUBTEXT);
        }
        else if (Fl_Hold_Browser* b = dynamic_cast<Fl_Hold_Browser*>(w)) {
            b->color(CLR_CARD); b->textcolor(CLR_TEXT);
        }
        else if (Fl_Box* bx = dynamic_cast<Fl_Box*>(w)) {
            if (bx->label()) bx->labelcolor(CLR_TEXT);
        }
        w->redraw();
    }
}

void restyle_window_recursively(Fl_Widget* w) {
    if (!w) return;
    if (Fl_Group* g = dynamic_cast<Fl_Group*>(w)) {
        restyle_group(g);
        return;
    }
    w->color(CLR_BG);
    w->redraw();
}

// FINANCE MANAGER (Backend)
class FinanceManager {
    vector<Transaction> transactions;
    map<string, double> categoryBudgets;
    string dbFile, budgetFile, username;
public:
    FinanceManager(const string &user) {
        username = Utils::sanitizeFilename(user);
        dbFile = "finance_db_" + username + ".txt";
        budgetFile = "budget_db_" + username + ".txt";
        loadTransactions(); loadBudgets();
    }
    void addTransaction(const string &type, double amount, const string &category, const string &note, const string &date) {
        if(amount <= 0) throw runtime_error("Amount must be positive.");
        if(category.empty()) throw runtime_error("Category is required.");
        transactions.emplace_back(type, amount, category, note, date);
        saveTransactions();
    }
    void editTransaction(const string &id, const string &type, double amount, const string &category, const string &note, const string &date) {
        for(auto &t : transactions) {
            if(t.getId() == id) {
                t.setType(type); t.setAmount(amount); t.setCategory(category); t.setNote(note); t.setDate(date);
                saveTransactions(); return;
            }
        }
    }
    void deleteTransaction(const string &id) {
        transactions.erase(remove_if(transactions.begin(), transactions.end(), [&](const Transaction &t){ return t.getId() == id; }), transactions.end());
        saveTransactions();
    }
    const vector<Transaction>& getTransactions() const { return transactions; }
    vector<Transaction> getFilteredTransactions(const string &typeFilter) {
        if(typeFilter == "All") return transactions;
        vector<Transaction> filtered;
        for(auto &t : transactions) if(t.getType() == typeFilter) filtered.push_back(t);
        return filtered;
    }
    void getBalanceInfo(double &income, double &expense, double &balance) {
        income = expense = 0;
        for(auto &t : transactions) {
            if(t.getType() == "Income") income += t.getAmount();
            else expense += t.getAmount();
        }
        balance = income - expense;
    }
    void setBudget(const string &category, double limit) {
        categoryBudgets[category] = limit; saveBudgets();
    }
    vector<Budget> getBudgetStatus() {
        vector<Budget> result;
        map<string,double> spent;
        for(auto &t : transactions) if(t.getType() == "Expense") spent[t.getCategory()] += t.getAmount();
        for(auto &b : categoryBudgets) result.push_back({b.first, b.second, spent[b.first]});
        return result;
    }
    vector<string> getAvailableYears() {
        set<string> years;
        years.insert(Utils::getCurrentDate().substr(0,4));
        for(auto &t : transactions) if(t.getDate().length()>=4) years.insert(t.getDate().substr(0,4));
        return vector<string>(years.rbegin(), years.rend());
    }
    ReportStats getReportForPeriod(const string& year, int monthIndex, vector<Transaction> &periodTrans) {
        ReportStats stats = {0, 0, 0};
        periodTrans.clear();
        string monthStr = (monthIndex < 10 ? "0" : "") + to_string(monthIndex);
        for(auto &t : transactions) {
            if(t.getDate().length() < 7) continue;
            string tYear = t.getDate().substr(0,4);
            string tMonth = t.getDate().substr(5,2);
            if(tYear == year && (monthIndex == 0 || tMonth == monthStr)) {
                if(t.getType() == "Income") stats.income += t.getAmount();
                else stats.expense += t.getAmount();
                periodTrans.push_back(t);
            }
        }
        stats.savings = stats.income - stats.expense;
        return stats;
    }
    void exportCSV() {
        string fname = "finance_export_" + username + ".csv";
        ofstream file(fname);
        file << "ID,Date,Type,Category,Amount,Note\n";
        for(auto &t : transactions) {
            string cat = t.getCategory(); replace(cat.begin(), cat.end(), ',', ';');
            string note = t.getNote(); replace(note.begin(), note.end(), ',', ';');
            file << t.getId() << "," << t.getDate() << "," << t.getType() << "," << cat << "," << t.getAmount() << "," << note << "\n";
        }
        file.close();
    }
private:
    void saveTransactions() {
        ofstream file(dbFile);
        for(auto &t : transactions) file << t.getId() << "|" << t.getType() << "|" << t.getAmount() << "|" << t.getCategory() << "|" << t.getNote() << "|" << t.getDate() << "\n";
    }
    void loadTransactions() {
        transactions.clear(); ifstream file(dbFile); if(!file.is_open()) return;
        string line;
        while(getline(file,line)) {
            if(line.empty()) continue;
            stringstream ss(line);
            string id,type,cat,note,date,amtStr; double amt=0;
            getline(ss,id,'|'); getline(ss,type,'|'); getline(ss,amtStr,'|'); getline(ss,cat,'|'); getline(ss,note,'|'); getline(ss,date,'|');
            try { amt = stod(amtStr); } catch(...) { amt = 0; }
            transactions.emplace_back(type,amt,cat,note,date,id);
        }
    }
    void saveBudgets() {
        ofstream file(budgetFile); for(auto &b : categoryBudgets) file << b.first << "|" << b.second << "\n";
    }
    void loadBudgets() {
        categoryBudgets.clear(); ifstream file(budgetFile); if(!file.is_open()) return;
        string line;
        while(getline(file,line)) {
            if(line.empty()) continue; size_t pos = line.find('|');
            if(pos!=string::npos) { categoryBudgets[line.substr(0,pos)] = stod(line.substr(pos+1)); }
        }
    }
};

// EDIT DIALOG
class EditDialog : public Fl_Double_Window {
public:
    Fl_Choice *inputType;
    Fl_Float_Input *inputAmount;
    Fl_Input *inputCategory, *inputNote, *inputDate;
    Fl_Button *btnSave,*btnCancel;
    bool saved=false;
    EditDialog() : Fl_Double_Window(340,420,"Edit Transaction") {
        color(CLR_BG);
        set_modal();

        int mx = 20, mw = 300;

        inputType = new Fl_Choice(mx, 40, mw, 30, "Type");
        inputType->align(FL_ALIGN_TOP_LEFT); inputType->add("Income"); inputType->add("Expense");
        inputType->box(FL_FLAT_BOX); inputType->color(CLR_INPUT_BG); inputType->textcolor(CLR_TEXT); inputType->labelcolor(CLR_SUBTEXT);

        inputAmount = new Fl_Float_Input(mx, 100, mw, 30, "Amount");
        styleInput(inputAmount); inputAmount->align(FL_ALIGN_TOP_LEFT);

        inputCategory = new Fl_Input(mx, 160, mw, 30, "Category");
        styleInput(inputCategory); inputCategory->align(FL_ALIGN_TOP_LEFT);

        inputNote = new Fl_Input(mx, 220, mw, 30, "Note");
        styleInput(inputNote); inputNote->align(FL_ALIGN_TOP_LEFT);

        inputDate = new Fl_Input(mx, 280, mw, 30, "Date (YYYY-MM-DD)");
        styleInput(inputDate); inputDate->align(FL_ALIGN_TOP_LEFT);

        btnSave = new Fl_Button(mx, 350, 140, 35, "Save Changes");
        styleButton(btnSave, CLR_PRIMARY);
        btnSave->callback([](Fl_Widget*,void* v){ ((EditDialog*)v)->saved=true; ((EditDialog*)v)->hide(); },this);

        btnCancel = new Fl_Button(180, 350, 140, 35, "Cancel");
        styleButton(btnCancel, fl_rgb_color(80, 80, 90));
        btnCancel->callback([](Fl_Widget* w,void* v){ ((EditDialog*)v)->hide(); },this);

        end();
    }
    void loadData(Transaction &t) {
        inputType->value(t.getType()=="Income"?0:1);
        char buf[64]; sprintf(buf,"%.2f",t.getAmount()); inputAmount->value(buf);
        inputCategory->value(t.getCategory().c_str());
        inputNote->value(t.getNote().c_str());
        inputDate->value(t.getDate().c_str());
    }
};

// LOGIN WINDOW
class LoginWindow : public Fl_Double_Window {
public:
    Fl_Input *usernameInput;
    Fl_Input *passwordInput;
    Fl_Button *btnLogin;
    Fl_Button *btnRegister;
    string loggedUsername;
    bool success = false;

    LoginWindow() : Fl_Double_Window(400, 350, "Pro Finance Login") {
        color(CLR_BG);

        Fl_Box *header = new Fl_Box(0, 30, 400, 50, "Welcome Back");
        header->labelfont(FL_HELVETICA_BOLD);
        header->labelsize(24);
        header->labelcolor(CLR_TEXT);

        Fl_Group *card = new Fl_Group(40, 100, 320, 220);
        card->box(FL_RFLAT_BOX);
        card->color(CLR_CARD);
        card->begin();

        usernameInput = new Fl_Input(70, 140, 260, 35, "Username");
        styleInput(usernameInput); usernameInput->align(FL_ALIGN_TOP_LEFT);

        passwordInput = new Fl_Input(70, 210, 260, 35, "Password");
        styleInput(passwordInput); passwordInput->align(FL_ALIGN_TOP_LEFT);
        passwordInput->type(FL_SECRET_INPUT);

        btnLogin = new Fl_Button(70, 270, 120, 35, "Login");
        styleButton(btnLogin, CLR_PRIMARY);
        btnLogin->callback([](Fl_Widget* w, void* data){ ((LoginWindow*)data)->login(); }, this);

        btnRegister = new Fl_Button(210, 270, 120, 35, "Register");
        styleButton(btnRegister, CLR_SUCCESS);
        btnRegister->callback([](Fl_Widget* w, void* data){ ((LoginWindow*)data)->registerUser(); }, this);

        card->end();
        end();
    }

    void login() {
        ifstream file("users.txt");
        string line, user=usernameInput->value(), pass=passwordInput->value();
        while (getline(file, line)) {
            stringstream ss(line); string u, p;
            getline(ss, u, '|'); getline(ss, p, '|');
            if (u == user && p == pass) {
                loggedUsername = user; success = true; hide(); return;
            }
        }
        fl_alert("Invalid username or password!");
    }
    void registerUser() {
        string user=usernameInput->value(), pass=passwordInput->value();
        if(user.empty() || pass.empty()) { fl_alert("Fields cannot be empty!"); return; }
        ifstream check("users.txt"); string line;
        while(getline(check, line)) { if(line.substr(0, line.find('|')) == user) { fl_alert("User exists!"); return; } }

        ofstream file("users.txt", ios::app); file << user << "|" << pass << "\n";
        string safe = Utils::sanitizeFilename(user);
        ofstream f("finance_db_" + safe + ".txt", ios::app);
        ofstream b("budget_db_" + safe + ".txt", ios::app);
        fl_message("Registered! Please login.");
    }
};

// MAIN APP WINDOW
class FinanceApp : public Fl_Double_Window {
    FinanceManager *manager;
    Fl_Tabs *tabs;
    Fl_Group *grpAdd,*grpHistory,*grpBudget,*grpDashboard,*grpMonthly;

    Fl_Choice *inputType;
    Fl_Float_Input *inputAmount;
    Fl_Input *inputCategory,*inputNote,*inputDate;

    Fl_Choice *filterType;
    Fl_Hold_Browser *browser;
    vector<string> displayedIds;

    Fl_Input *inputBudgetCat;
    Fl_Float_Input *inputBudgetLimit;
    Fl_Hold_Browser *budgetBrowser;

    PieChart *chart;
    Fl_Box *boxInc,*boxExp,*boxBal;
    Fl_Multiline_Output *aiBox;

    Fl_Choice *choiceYear, *choiceMonth;
    Fl_Box *lblInc, *lblExp, *lblSave;
    Fl_Hold_Browser *monthlyBrowser;

public:
   
    FinanceApp(const string &username) : Fl_Double_Window(800, 600, "Pro Finance Manager") {
        color(CLR_BG);
        manager = new FinanceManager(username);

       
        tabs = new Fl_Tabs(0, 0, 800, 600); 
        tabs->color(fl_rgb_color(35, 38, 45));
        tabs->selection_color(CLR_PRIMARY);
        tabs->labelcolor(CLR_SUBTEXT);
        tabs->box(FL_FLAT_BOX);

        const int CONTENT_Y = 30;
        const int CONTENT_H = 570; 

        //DASHBOARD
        grpDashboard = new Fl_Group(0, CONTENT_Y, 800, CONTENT_H, "Dashboard");
        grpDashboard->color(CLR_BG);
        CardGroup *cardChart = new CardGroup(20, 50, 240, 260);
        cardChart->begin();
            Fl_Box *t1 = new Fl_Box(20, 60, 240, 30, "Cash Flow");
            t1->labelcolor(CLR_TEXT); t1->labelfont(FL_HELVETICA_BOLD);
            chart = new PieChart(40, 95, 200, 200);
        cardChart->end();

        int statW = 240;
        auto makeStat = [&](int x, int y, Fl_Box*& b, const char* title, Fl_Color c) {
            Fl_Group* g = new Fl_Group(x, y, statW, 80);
            g->box(FL_RFLAT_BOX); g->color(CLR_CARD);
            g->begin();
                Fl_Box *lbl = new Fl_Box(x+10, y+5, statW-20, 20, title);
                lbl->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE); lbl->labelsize(10); lbl->labelcolor(CLR_SUBTEXT);
                b = new Fl_Box(x+10, y+25, statW-20, 30, "$0.00");
                b->labelfont(FL_HELVETICA_BOLD); b->labelsize(20); b->labelcolor(c); b->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
            g->end();
        };

        makeStat(280, 50, boxInc, "TOTAL INCOME", CLR_SUCCESS);
        makeStat(280, 140, boxExp, "TOTAL EXPENSE", CLR_DANGER);
        makeStat(280, 230, boxBal, "CURRENT BALANCE", CLR_PRIMARY);

        CardGroup *cardAI = new CardGroup(540, 50, 240, 260);
        cardAI->begin();
            Fl_Box *aiTitle = new Fl_Box(540, 60, 240, 30, "AI Insights");
            aiTitle->labelfont(FL_HELVETICA_BOLD); aiTitle->labelcolor(CLR_TEXT);
            aiBox = new Fl_Multiline_Output(550, 90, 220, 210);
            aiBox->box(FL_NO_BOX); aiBox->textsize(12); aiBox->color(CLR_CARD); aiBox->textcolor(CLR_SUBTEXT);
        cardAI->end();
        grpDashboard->end();

        //ADD TRANSACTION
        grpAdd = new Fl_Group(0, CONTENT_Y, 800, CONTENT_H, "New Transaction");
        grpAdd->color(CLR_BG);
        CardGroup *formCard = new CardGroup(200, 60, 400, 450);
        formCard->begin();
            Fl_Box *frmTitle = new Fl_Box(200, 70, 400, 40, "Add Record");
            frmTitle->labelfont(FL_HELVETICA_BOLD); frmTitle->labelsize(18); frmTitle->labelcolor(CLR_TEXT);

            int fx = 250, fw = 300;
            inputType = new Fl_Choice(fx, 130, fw, 30, "Type");
            inputType->align(FL_ALIGN_TOP_LEFT); inputType->add("Income"); inputType->add("Expense"); inputType->value(0);
            inputType->box(FL_FLAT_BOX); inputType->color(CLR_INPUT_BG); inputType->textcolor(CLR_TEXT); inputType->labelcolor(CLR_SUBTEXT);

            inputAmount = new Fl_Float_Input(fx, 190, fw, 30, "Amount ($)");
            styleInput(inputAmount); inputAmount->align(FL_ALIGN_TOP_LEFT);

            inputCategory = new Fl_Input(fx, 250, fw, 30, "Category");
            styleInput(inputCategory); inputCategory->align(FL_ALIGN_TOP_LEFT);

            inputNote = new Fl_Input(fx, 310, fw, 30, "Note");
            styleInput(inputNote); inputNote->align(FL_ALIGN_TOP_LEFT);

            inputDate = new Fl_Input(fx, 370, fw, 30, "Date");
            styleInput(inputDate); inputDate->align(FL_ALIGN_TOP_LEFT); inputDate->value(Utils::getCurrentDate().c_str());

            Fl_Button *btnAdd = new Fl_Button(fx, 430, fw, 40, "Save Record");
            styleButton(btnAdd, CLR_PRIMARY);
            btnAdd->callback([](Fl_Widget*, void* v){ ((FinanceApp*)v)->addTransaction(); }, this);
        formCard->end();
        grpAdd->end();

        //HISTORY 
        grpHistory = new Fl_Group(0, CONTENT_Y, 800, CONTENT_H, "Transactions");
        grpHistory->color(CLR_BG);

        filterType = new Fl_Choice(20, 50, 150, 30);
        filterType->add("All"); filterType->add("Income"); filterType->add("Expense"); filterType->value(0);
        filterType->box(FL_FLAT_BOX); filterType->color(CLR_CARD); filterType->textcolor(CLR_TEXT);
        filterType->callback([](Fl_Widget*, void* v){ ((FinanceApp*)v)->refreshHistory(); }, this);

        Fl_Button *btnExp = new Fl_Button(630, 50, 150, 30, "Export CSV");
        styleButton(btnExp, CLR_SUCCESS);
        btnExp->callback([](Fl_Widget*, void* v){ ((FinanceApp*)v)->manager->exportCSV(); fl_message("Exported!"); }, this);

        browser = new Fl_Hold_Browser(20, 90, 760, 400);
        browser->box(FL_FLAT_BOX); browser->color(CLR_CARD); browser->textfont(FL_COURIER); browser->textsize(13);
        browser->selection_color(fl_rgb_color(60,60,70)); browser->textcolor(CLR_TEXT);

        Fl_Button *btnEdit = new Fl_Button(20, 500, 120, 30, "Edit Selected");
        styleButton(btnEdit, fl_rgb_color(243, 156, 18));
        btnEdit->callback([](Fl_Widget*, void* v){ ((FinanceApp*)v)->editTransaction(); }, this);

        Fl_Button *btnDel = new Fl_Button(150, 500, 120, 30, "Delete Selected");
        styleButton(btnDel, CLR_DANGER);
        btnDel->callback([](Fl_Widget*, void* v){ ((FinanceApp*)v)->deleteTransaction(); }, this);
        grpHistory->end();

        //BUDGET
        grpBudget = new Fl_Group(0, CONTENT_Y, 800, CONTENT_H, "Budgeting");
        grpBudget->color(CLR_BG);

        CardGroup *bgInput = new CardGroup(20, 50, 760, 80);
        bgInput->begin();
            inputBudgetCat = new Fl_Input(40, 75, 200, 30, "Category Name");
            styleInput(inputBudgetCat); inputBudgetCat->align(FL_ALIGN_TOP_LEFT);

            inputBudgetLimit = new Fl_Float_Input(260, 75, 150, 30, "Limit ($)");
            styleInput(inputBudgetLimit); inputBudgetLimit->align(FL_ALIGN_TOP_LEFT);

            Fl_Button *btnSet = new Fl_Button(430, 75, 120, 30, "Set Limit");
            styleButton(btnSet, CLR_PRIMARY);
            btnSet->callback([](Fl_Widget*, void* v){ ((FinanceApp*)v)->setBudget(); }, this);
        bgInput->end();

        budgetBrowser = new Fl_Hold_Browser(20, 150, 760, 380);
        budgetBrowser->box(FL_FLAT_BOX); budgetBrowser->color(CLR_CARD);
        budgetBrowser->textcolor(CLR_TEXT); budgetBrowser->textfont(FL_COURIER);
        grpBudget->end();

        // --- MONTHLY REPORT ---
        grpMonthly = new Fl_Group(0, CONTENT_Y, 800, CONTENT_H, "Monthly Reports");
        grpMonthly->color(CLR_BG);

        CardGroup *ctrlBar = new CardGroup(20, 50, 760, 60);
        ctrlBar->begin();
            choiceYear = new Fl_Choice(40, 65, 100, 30); choiceYear->box(FL_FLAT_BOX); choiceYear->color(CLR_INPUT_BG); choiceYear->textcolor(CLR_TEXT); choiceYear->labelcolor(CLR_SUBTEXT);
            choiceMonth = new Fl_Choice(160, 65, 120, 30);
            choiceMonth->box(FL_FLAT_BOX); choiceMonth->color(CLR_INPUT_BG); choiceMonth->textcolor(CLR_TEXT); choiceMonth->labelcolor(CLR_SUBTEXT);
            choiceMonth->add("All");
            choiceMonth->add("01 - Jan"); choiceMonth->add("02 - Feb"); choiceMonth->add("03 - Mar");
            choiceMonth->add("04 - Apr"); choiceMonth->add("05 - May"); choiceMonth->add("06 - Jun");
            choiceMonth->add("07 - Jul"); choiceMonth->add("08 - Aug"); choiceMonth->add("09 - Sep");
            choiceMonth->add("10 - Oct"); choiceMonth->add("11 - Nov"); choiceMonth->add("12 - Dec");
            choiceMonth->value(0);

            Fl_Button *btnFilterM = new Fl_Button(300, 65, 120, 30, "Analyze");
            styleButton(btnFilterM, CLR_PRIMARY);
            btnFilterM->callback([](Fl_Widget*, void* v){ ((FinanceApp*)v)->filterMonthlyReport(); }, this);
        ctrlBar->end();

        int cw = 240;
        auto makeRepCard = [&](int x, Fl_Box*& b, const char* t, Fl_Color c){
             Fl_Group *g = new Fl_Group(x, 120, cw, 60);
             g->box(FL_RFLAT_BOX); g->color(CLR_CARD);
             b = new Fl_Box(x+10, 125, cw-20, 50, "0.00");
             b->labelcolor(c); b->labelfont(FL_HELVETICA_BOLD);
             g->end();
        };

        makeRepCard(20, lblInc, "Income", CLR_SUCCESS);
        makeRepCard(280, lblExp, "Expense", CLR_DANGER);
        makeRepCard(540, lblSave, "Savings", CLR_PRIMARY);

        monthlyBrowser = new Fl_Hold_Browser(20, 200, 760, 330);
        monthlyBrowser->box(FL_FLAT_BOX); monthlyBrowser->color(CLR_CARD);
        monthlyBrowser->textcolor(CLR_TEXT); monthlyBrowser->textfont(FL_COURIER);

        grpMonthly->end();

        tabs->end();
        resizable(tabs);

        updateYearDropdown();
        refreshHistory(); refreshBudget(); refreshDashboard();
    }

    void addTransaction() {
        try {
            manager->addTransaction(inputType->text(), atof(inputAmount->value()), inputCategory->value(), inputNote->value(), inputDate->value());
            inputAmount->value(""); inputCategory->value(""); inputNote->value(""); inputDate->value(Utils::getCurrentDate().c_str());
            refreshHistory(); refreshDashboard(); updateYearDropdown();
            fl_message("Transaction Saved!");
        } catch(exception &e) { fl_alert(e.what()); }
    }

    void refreshHistory() {
        browser->clear(); displayedIds.clear();
        vector<Transaction> trs = manager->getFilteredTransactions(filterType->text());
        char line[256];
        for(auto &t : trs) {
            snprintf(line, sizeof(line), "%-12s | $%-10.2f | %-15s | %s", t.getDate().c_str(), t.getAmount(), t.getCategory().c_str(), t.getNote().c_str());
            browser->add(line);
            displayedIds.push_back(t.getId());
        }
        browser->redraw();
    }

    void refreshDashboard() {
        double inc, exp, bal; manager->getBalanceInfo(inc,exp,bal);
        chart->updateValues(inc,exp);
        char buf[64];
        snprintf(buf, sizeof(buf), "$%.2f", inc); boxInc->copy_label(buf);
        snprintf(buf, sizeof(buf), "$%.2f", exp); boxExp->copy_label(buf);
        snprintf(buf, sizeof(buf), "$%.2f", bal); boxBal->copy_label(buf);
        generateAISuggestions();
        chart->redraw();
    }

    void refreshBudget() {
        budgetBrowser->clear(); char line[256];
        for(auto &b : manager->getBudgetStatus()) {
            snprintf(line, sizeof(line), "%-15s | Limit: $%-8.2f | Spent: $%-8.2f | Left: $%.2f", b.category.c_str(), b.limit, b.spent, (b.limit-b.spent));
            budgetBrowser->add(line);
        }
        budgetBrowser->redraw();
    }

    void updateYearDropdown() {
        choiceYear->clear();
        for(const string &y : manager->getAvailableYears()) choiceYear->add(y.c_str());
        if(choiceYear->size()>0) choiceYear->value(0);
    }

    void filterMonthlyReport() {
        monthlyBrowser->clear();
        if(choiceYear->value() < 0) return;
        vector<Transaction> periodTrans;
        ReportStats stats = manager->getReportForPeriod(choiceYear->text(), choiceMonth->value(), periodTrans);

        static char bInc[64], bExp[64], bSav[64];
        snprintf(bInc, sizeof(bInc), "Income\n$%.2f", stats.income); lblInc->copy_label(bInc);
        snprintf(bExp, sizeof(bExp), "Expense\n$%.2f", stats.expense); lblExp->copy_label(bExp);
        snprintf(bSav, sizeof(bSav), "Savings\n$%.2f", stats.savings); lblSave->copy_label(bSav);

        char line[256];
        for(auto &t : periodTrans) {
            snprintf(line, sizeof(line), "%s | %-8s | $%-8.2f | %s", t.getDate().c_str(), t.getType().c_str(), t.getAmount(), t.getCategory().c_str());
            monthlyBrowser->add(line);
        }
        if(periodTrans.empty()) monthlyBrowser->add("No records found.");
        monthlyBrowser->redraw();
    }

    void editTransaction() {
        int idx = browser->value()-1; if(idx<0) return;
        string selected_id = displayedIds[idx];
        Transaction* t_ptr = nullptr;
        for (auto& t : const_cast<vector<Transaction>&>(manager->getTransactions())) {
            if (t.getId() == selected_id) {
                t_ptr = &t;
                break;
            }
        }
        if (!t_ptr) return;

        EditDialog dlg;
        dlg.loadData(*t_ptr);
        dlg.show();
        while(dlg.shown()) Fl::wait();
        if(dlg.saved) {
            manager->editTransaction(selected_id, dlg.inputType->text(), atof(dlg.inputAmount->value()), dlg.inputCategory->value(), dlg.inputNote->value(), dlg.inputDate->value());
            refreshHistory(); refreshDashboard(); updateYearDropdown();
        }
    }

    void deleteTransaction() {
        int idx = browser->value()-1; if(idx<0) return;
        if(fl_choice("Are you sure you want to delete this record?","No","Yes",0)==1) {
            manager->deleteTransaction(displayedIds[idx]);
            refreshHistory(); refreshDashboard(); updateYearDropdown();
        }
    }

    void setBudget() {
        double lim = atof(inputBudgetLimit->value());
        if(strlen(inputBudgetCat->value())>0 && lim>0) {
            manager->setBudget(inputBudgetCat->value(), lim);
            inputBudgetCat->value(""); inputBudgetLimit->value("");
            refreshBudget();
        }
    }

    void generateAISuggestions() {
        stringstream ss; double inc, exp, bal; manager->getBalanceInfo(inc,exp,bal);
        if(bal < 0) ss << "Warning: You are spending more than you earn.\n";
        else if(bal < inc*0.2) ss << "Tip: Try to save at least 20% of your income.\n";
        else ss << "Great job! Your savings rate is healthy.\n";

        for(const auto &b : manager->getBudgetStatus()) {
            if(b.spent > b.limit) ss << "! Over budget in " << b.category << "\n";
        }
        aiBox->value(ss.str().c_str());
    }

    void restyle_app_window() {
        color(CLR_BG);
        redraw();
        restyle_window_recursively(this);
    }
};

// MAIN FUNCTION
int main() {
    srand((unsigned int)time(0));
    Fl::scheme("gtk+");
    Fl::set_font(FL_HELVETICA, "Arial");

    set_theme();

    LoginWindow loginWin;
    loginWin.show();
    while(loginWin.shown()) Fl::wait();

    if (loginWin.success) {
        FinanceApp app(loginWin.loggedUsername);
        app.show();
        return Fl::run();
    }
    return 0;
}