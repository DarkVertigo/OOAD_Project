#include <sqlite3.h>
#include <chrono>
#include <ctime>
//#include "header.h"
#include "checks.h"
#include "PasswordGenerator.h"
#include "forgotPassword.h"

typedef struct systemState
{
    int shopKeeperCount, deliveryPersonCount, CustomerCount;
    int orderCount, productCount;

} systemState;
systemState state;

string temporaryID;       // also helps in adding transactions
profile temporaryProfile; // helps in editing profile
string temporaryPassword;
map<string,string> ShopKeeperId_to_name;
string add;
int temporaryOrderID; //used in assigning the order to the delivery person

class admin
{
public:
    sqlite3 *DB;
    int exit;
    string data;
    char *messaggeError;

    int shopKeeperCount, deliveryPersonCount, CustomerCount; // helps in assigning the ID to everyone
    product *global_inventory_array;
    map<string, set<int>> global_inven_map; //mapping from product name to product
    //this is comment static product* personal_inventory;
    fstream global_inve_file;
    //ifstream personal_inventory_file;
    map<string, set<int>> personal_inventory; // shopkeeper id mapped to vector of productsID owned by him
    map<int, product> productId_to_product;

    static int callback(void *data, int argc, char **argv, char **azColName)
    {
        for (int i = 0; i < argc; i++)
            printf("%s = %s", azColName[i], argv[i] ? argv[i] : "NULL");
        return 0;
    }

    static int get_ID(void *data, int argc, char **argv, char **azColName)
    {
        temporaryID = argv[0] ? argv[0] : "#";
        return 0;
    }

    static int get_information(void *data, int argc, char **argv, char **azColName)
    {
        temporaryProfile.name = argv[1];
        temporaryProfile.surname = argv[2];
        temporaryProfile.email = argv[3];
        temporaryProfile.address = argv[4];
        temporaryProfile.contact = argv[5];
        temporaryProfile.username = argv[6];
        temporaryProfile.password = argv[7];
        return 0;
    }
        void createDataBase()
    {
        int exit = 0;
        shopKeeperCount = deliveryPersonCount = CustomerCount = 0;
        exit = sqlite3_open("userDatabase.db", &DB);
        if (exit)
        {
            cerr << "Error open DB " << sqlite3_errmsg(DB) << endl;
            std::exit(1);
        }
        else
            std::cout << "Created Database Successfully!" << std::endl;
        string sql = "CREATE TABLE PERSON("
                     "ID TEXT PRIMARY KEY     NOT NULL, "
                     "NAME    TEXT    NOT NULL, "
                     "SURNAME   TEXT     NOT NULL, "
                     "EMAIL_ID   TEXT     NOT NULL, "
                     "ADDRESS    CHAR(50), "
                     "CONTACT_NO   TEXT  NOT NULL,"
                     "USERNAME    TEXT   NOT NULL,"
                     "PASSWORD   TEXT   NOT NULL);";
        string sql2 = "CREATE TABLE USER_TRANSACTION("
                      "ID TEXT PRIMARY KEY     NOT NULL, "
                      "TRANSACTIONS  TEXT  NOT NULL );";
        string sql3 = "CREATE TABLE USER_MAP("
                      "USERNAME TEXT PRIMARY KEY     NOT NULL, "
                      "ID  TEXT  NOT NULL );";
        string sql4 = "CREATE TABLE BLACKLISTED("
                      "USERNAME TEXT PRIMARY KEY     NOT NULL );";
        string sql5 = "CREATE TABLE ASSIGNED_ORDER("
                            "ID TEXT PRIMARY KEY     NOT NULL, "
                            "ORDER_ID  TEXT  NOT NULL );"; 
        string sql5 = "CREATE TABLE WISHLIST("
                            "ID TEXT PRIMARY KEY     NOT NULL, "
                            "ITEMS  TEXT  NOT NULL );"; 
        exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messaggeError);
        exit = sqlite3_exec(DB, sql2.c_str(), NULL, 0, &messaggeError);
        exit = sqlite3_exec(DB, sql3.c_str(), NULL, 0, &messaggeError);
        exit = sqlite3_exec(DB, sql4.c_str(), NULL, 0, &messaggeError);
        exit = sqlite3_exec(DB, sql5.c_str(), NULL, 0, &messaggeError);
        if (exit == SQLITE_OK)
        {
            cerr << "Error Create Table" << endl;
            sqlite3_free(messaggeError);
        }
        else
            cout << "Table created Successfully" << endl;
    }

    static int get_name(void *data, int argc, char **argv, char **azColName)
    {
        temporaryID = argv[1];
        return 0;
    }

    string nameFromId(string id){
        temporaryID="#";
        string query = "SELECT * FROM PERSON WHERE ID = \'" + id + "\';";
        int rc=sqlite3_exec(DB, query.c_str(), get_name, NULL, NULL);
        if(rc!=SQLITE_OK) cout<<"Error select" << "\n" ;
        else cout<<"Operation OK" << "\n" ;
        return temporaryID ;
    }

    void loadDatabase()
    {
        createDataBase();

        global_inve_file.open("global_inventory_db", ios::in);

        if (!global_inve_file.is_open())
        {
             global_inve_file.open("global_inventory_db", ios::out);
             global_inve_file.close();
            global_inve_file.open("global_inventory_db", ios::in);
        }
        ifstream sysfile;
        sysfile.open("systemState",ios::in);
        sysfile.read((char*)&state,sizeof(systemState));
        sysfile.close();
        global_inve_file.seekg(0, ios::end);
        int fileSize = global_inve_file.tellg();
        cout<<fileSize<<endl;
        global_inve_file.seekg(0, ios::beg);
        int size = fileSize / sizeof(product);
        global_inventory_array = (product *)malloc(size * sizeof(product));
        cout<<global_inve_file.tellg()<<endl;
        global_inve_file.read((char *)global_inventory_array, fileSize);
        cout<<"size "<<size<<endl;
        for (int i = 0; i < size; ++i)
        {
            global_inven_map[global_inventory_array[i].product_name].insert(global_inventory_array[i].product_id);
            personal_inventory[global_inventory_array[i].shopkeeper_id].insert(global_inventory_array[i].product_id);
            productId_to_product[global_inventory_array[i].product_id]=(global_inventory_array[i]);
        }
        global_inve_file.close();
        /*personal_inventory_file.open("personal_inventory_db", ios::binary);
        product *dummy = (product *)malloc(size);
        personal_inventory_file.read((char *)dummy, size * sizeof(product));
        int i = 0;
        while (i < size)
        {
            product *tmp;
            tmp = dummy + i;
            if (personal_inventory.find(string(tmp->shopkeeper_id)) != personal_inventory.end())
            {
                product p = *tmp;
                personal_inventory[string(tmp->shopkeeper_id)].push_back(p);
            }
        }*/
    }

    string Insert(string name, string surname, string email, string address, string username, string password, string contact, enum typeOfUser type)
    {
        string id = "";
        if (type == Customer)
        {
            id = 'C' + to_string(CustomerCount++);
        }
        else if (type == deliveryPerson)
        {
            id = 'D' + to_string(deliveryPersonCount++);
        }
        else
        {
            id = 'S' + to_string(shopKeeperCount++);
        }
        string temp1 = '\'' + id + "\',\'" + name + "\',\'" + surname + "\',\'" + email + "\',\'" + address + "\',\'" + contact + "\',\'" + username + "\',\'" + password + '\'';
        string temp = '\'' + username + '\'' + ',' + '\'' + id + '\'';
        string temp2 = '\'' + id + "\',\'\'";
        string sql("INSERT INTO PERSON VALUES(" + temp1 + ");");
        string sql1("INSERT INTO USER_MAP VALUES(" + temp + ");");
        string sql2("INSERT INTO USER_TRANSACTION VALUES(" + temp2 + ");");
        exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messaggeError);
        if (exit != SQLITE_OK)
        {
            cerr << "Error Insert    " << messaggeError << endl;
            sqlite3_free(messaggeError);
        }
        else
            cout << "Record inserted Successfully!" << endl;

        exit = sqlite3_exec(DB, sql1.c_str(), NULL, 0, &messaggeError);
        if (exit != SQLITE_OK)
        {
            cerr << "Error Insert   " << messaggeError << endl;
            sqlite3_free(messaggeError);
        }
        else
            cout << "Record inserted Successfully!" << endl;

        exit = sqlite3_exec(DB, sql2.c_str(), NULL, 0, &messaggeError);
        if (exit != SQLITE_OK)
        {
            cerr << "Error Insert   " << messaggeError << endl;
            sqlite3_free(messaggeError);
        }
        else
            cout << "Record inserted Successfully!" << endl;
        return id;
    }

    profile authenticate(string username, string password)
    {
        string query = "SELECT ID FROM USER_MAP WHERE USERNAME = \'" + username + "\';";
        int rc = sqlite3_exec(DB, query.c_str(), get_ID, NULL, NULL);
        if (rc != SQLITE_OK || temporaryID == "#" ||isBlackListed(username))
        {
            //cerr << "Error SELECT" << endl;
            printHeader();
            cout <<fgred<< "\t\t\t\t\t\t\t\t\t\tINCORRECT Username or Password!!!!" << endl;
            temporaryProfile.name = "#";
            return temporaryProfile;
        }
        else
        {
            logStream << "Operation OK!" << endl;
        }
        temporaryProfile.id = temporaryID;
        query = "SELECT * FROM PERSON WHERE ID = \'" + temporaryID + "\';";
        sqlite3_exec(DB, query.c_str(), get_information, NULL, NULL);
        if (temporaryProfile.password != password)
        {
            delayBy(0);
            printHeader();
           cout <<fgred<< "\t\t\t\t\t\t\t\t     INCORRECT Username or Password" << endl;
           cout<<endl;
            temporaryProfile.name = "#";
            return temporaryProfile;
        }
        return temporaryProfile;
    }

    void deleteID(string id, string username)
    {
        string sql = "DELETE FROM PERSON WHERE ID = \'" + id + "\';";
        exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messaggeError);
        if (exit != SQLITE_OK)
        {
            cerr << "Error DELETE" << endl;
            sqlite3_free(messaggeError);
        }
        else
            cout << "Record deleted Successfully from PERSON!" << endl;
        sql = "DELETE FROM USER_TRANSACTION WHERE ID = \'" + id + "\';";
        exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messaggeError);
        if (exit != SQLITE_OK)
        {
            cerr << "Error DELETE" << endl;
            sqlite3_free(messaggeError);
        }
        else
            cout << "Record deleted Successfully from USER_TRANSACTION!" << endl;
        sql = "DELETE FROM USER_MAP WHERE USERNAME = \'" + username + "\';";
        exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messaggeError);
        if (exit != SQLITE_OK)
        {
            cerr << "Error DELETE" << endl;
            sqlite3_free(messaggeError);
        }
        else
            cout << "Record deleted Successfully from USER_MAP!" << endl;
    }

    void changeProfile(string id, string name, string surname, string email, string address, string username, string password, string contact)
    {
        string sql = "DELETE FROM PERSON WHERE ID = \'" + id + "\';";
        exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messaggeError);
        if (exit != SQLITE_OK)
        {
            logStream << "Error DELETE" << endl;
            sqlite3_free(messaggeError);
        }
        else
            logStream << "Record deleted Successfully!" << endl;
        string temp1 = '\'' + id + "\',\'" + name + "\',\'" + surname + "\',\'" + email + "\',\'" + address + "\',\'" + contact + "\',\'" + username + "\',\'" + password + '\'';
        string sql1("INSERT INTO PERSON VALUES(" + temp1 + ");");
        exit = sqlite3_exec(DB, sql1.c_str(), NULL, 0, &messaggeError);
        if (exit != SQLITE_OK)
        {
            logStream << "Error Insert" << endl;
            sqlite3_free(messaggeError);
        }
        else
            logStream << "Record inserted Successfully!" << endl;
    }

    void editProfile(string id)
    {
        string query = "SELECT * FROM PERSON WHERE ID = \'" + id + "\';";
        sqlite3_exec(DB, query.c_str(), get_information, NULL, NULL);
        char check;
        cout << "Do you wish to change your name?(Y/n) :: ";
        cin >> check;
        if (check == 'Y' || check == 'y')
        {
            string new_name;
            cin >> new_name;
            temporaryProfile.name = new_name;
        }
        cout << "Do you wish to change your Surname?(Y/n) :: ";
        cin >> check;
        if (check == 'Y' || check == 'y')
        {
            string new_surname;
            cin >> new_surname;
            temporaryProfile.surname = new_surname;
        }
        cout << "Do you wish to change your Email ID?(Y/n) :: ";
        cin >> check;
        if (check == 'Y' || check == 'y')
        {
            string new_email;
            cin >> new_email;
            while (!isEmailCorrect(new_email))
            {
                cout << "Enter a valid Email address(Only IIT Jodhpur official email addresses are considered valid) ";
                cin >> new_email;
            }
            temporaryProfile.email = new_email;
        }
        cout << "Do you wish to change your address?(Y/n) :: ";
        cin >> check;
        if (check == 'Y' || check == 'y')
        {
            string new_address;
            cin >> new_address;
            temporaryProfile.address = new_address;
        }
        cout << "Do you wish to change your Contact number?(Y/n) :: ";
        cin >> check;
        if (check == 'Y' || check == 'y')
        {
            string new_contact;
            cin >> new_contact;
            while (!isContactCorrect(new_contact))
            {
                cout << "Enter a valid contact number";
                cin >> new_contact;
            }
            temporaryProfile.contact = new_contact;
        }
        cout << "Do you wish to change your Password?(Y/n) :: ";
        cin >> check;
        if (check == 'Y' || check == 'y')
        {
            string new_password, confirm_new_password;
            cin >> new_password;
            cout << "Confirm Password: ";
            cin >> confirm_new_password;
            while (!(isPasswordCorrect(new_password) && confirm_new_password == new_password))
            {
                cout << "Enter a valid Password: ";
                cin >> new_password;
                cout << "Confirm Password: ";
                cin >> confirm_new_password;
            }
            temporaryProfile.password = new_password;
        }
        changeProfile(id, temporaryProfile.name, temporaryProfile.surname, temporaryProfile.email, temporaryProfile.address, temporaryProfile.username, temporaryProfile.password, temporaryProfile.contact);
    }
    static int update(void *data, int argc, char **argv, char **azColName)
    {
        int n = strlen(argv[0]) + add.size();
        int originalLength = strlen(argv[0]);
        char *tmp = (argv[0] + originalLength);
        tmp = (char *)malloc(sizeof(char) * n);
        for (int i = originalLength; i < n; ++i)
            argv[0][i] = add[i - originalLength];
        string sql = "UPDATE USER_TRANSACTION set TRANSACTIONS = \'" + string(argv[0]) + "\' WHERE ID = \'" + temporaryID + '\'';
        temporaryID = sql;
        return 0;
    }

    void addTransaction(string id, bool isPaid, int moneyTransferred, int orderID, string payment_mode, string timeOfOrder, string paymentUsing)
    {
        string temp = "Using ";
        if(payment_mode == "CASH ON DELIVERY")temp="";
        string query = "SELECT TRANSACTIONS FROM USER_TRANSACTION WHERE ID = \'" + id + "\';";
        add = to_string(orderID) + " | " + (isPaid ? "Paid" : "Refunded") + " | " + to_string(moneyTransferred) +\
              " | " + payment_mode + " | " + temp + paymentUsing + " | " + timeOfOrder+ "\n";
        temporaryID = id; 
        sqlite3_exec(DB, query.c_str(), update, NULL, NULL);
        exit = sqlite3_exec(DB, temporaryID.c_str(), callback, NULL, &messaggeError);
        if (exit != SQLITE_OK)
        {
            fprintf(stderr, "SQL error: %s\n", messaggeError);
            sqlite3_free(messaggeError);
        }
        else
        {
            fprintf(stdout, "Operation done successfully\n");
        }
    }


    void showTransaction(string id)
    {
        string query = "SELECT TRANSACTIONS FROM USER_TRANSACTION WHERE ID = \'" + id + "\';";
        int exit = sqlite3_exec(DB, query.c_str(), callback, NULL, NULL);
        if (exit != SQLITE_OK)
            cerr << "Error SELECT" << endl;
        else
        {
            cout << "Operation OK!" << endl;
        }
    }

    static int check_username(void *data, int argc, char **argv, char **azColName)
    {
        temporaryID = argv[0];
        return 0;
    }

    bool isUsernameTaken(string username){
        temporaryID = "#";
        string query = "SELECT * FROM USER_MAP WHERE USERNAME = \'" + username + "\';";
        sqlite3_exec(DB, query.c_str(), check_username, NULL, NULL); // only checks if username is taken or not i.e. returns 0 only if it is taken
        if (temporaryID != "#") return 1;
        else return 0;
    }

    string signUp(profile addToDatabase)
    {
        return Insert(addToDatabase.name, addToDatabase.surname, addToDatabase.email,
                   addToDatabase.address, addToDatabase.username, addToDatabase.password, addToDatabase.contact, addToDatabase.type);
    }

    void addToBlacklist(string username)
    {
        string sql("INSERT INTO BLACKLISTED VALUES(\'" + username + "\');");
        exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messaggeError);
        if (exit != SQLITE_OK)
        {
            cerr << "Error Insert   " << messaggeError << endl;
            sqlite3_free(messaggeError);
        }
        else
            cout << "Record inserted Successfully!" << endl;
    }

    bool isBlackListed(string username)
    {
        temporaryID = "#";
        string query = "SELECT * FROM BLACKLISTED WHERE USERNAME = \'" + username + "\';";
        exit = sqlite3_exec(DB, query.c_str(), check_username, NULL, NULL);
        return temporaryID != "#";
    }

    void dumpData()
    {
        global_inventory_array=(product*)malloc(productId_to_product.size()*sizeof(product));
        int index=0;
        for(auto i : productId_to_product)
        {
            global_inventory_array[index]=i.second;
            index++;
        }
        remove("global_inventory_db");
        global_inve_file.open("global_inventory_db",ios::out);
        global_inve_file.write((char*)global_inventory_array,productId_to_product.size()*sizeof(product));
        global_inve_file.close(); 
        remove("systemState");
        fstream outSysState;
        outSysState.open("systemState",ios::out);
        outSysState.write((char*)&state,sizeof(systemState));
        outSysState.close();
    }
    void insertProduct(product productToInsert)
    {
        //global_inve_file.open("global_inve")
        productId_to_product[productToInsert.product_id]=productToInsert;
    }
    void forgotPassword(string username){
        string new_password = PasswordGenerator();
        string message = "Your new Password is : "+new_password;
        string query = "SELECT ID FROM USER_MAP WHERE USERNAME = \'" + username + "\';";
        sqlite3_exec(DB, query.c_str(), get_ID, NULL, NULL);
        query = "SELECT * FROM PERSON WHERE ID = \'" + temporaryID + "\';";
        sqlite3_exec(DB, query.c_str(), get_information, NULL, NULL);
        temporaryProfile.password = new_password;
        sendPasswordToEmail(temporaryProfile.email,new_password);
        changeProfile(temporaryID, temporaryProfile.name,temporaryProfile.surname,\
        temporaryProfile.email,temporaryProfile.address,temporaryProfile.username,temporaryProfile.password,temporaryProfile.contact);
    }

    void assign_order(string id, int orderID){
        string temp = '\'' + id + "\',\'" + to_string(orderID)+'\'';string temp = '\'' + id + "\',\'" + to_string(orderID)+'\'';string temp = '\'' + id + "\',\'" + to_string(orderID)+'\'';        string sql("INSERT INTO ASSIGNED_ORDER VALUES(" + temp + ");");;
        exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messaggeError);
        if (exit != SQLITE_OK)
        {
            cerr << "Error Insert    " << messaggeError << endl;
            sqlite3_free(messaggeError);
        }
        else
            cout << "Record inserted Successfully!" << endl;
    }
 
    void finish_order(string id){
        string sql = "DELETE FROM ASSIGNED_ORDER WHERE ID = \'" + id + "\';";
        exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messaggeError);
        if (exit != SQLITE_OK)
        {
            cerr << "Error DELETE" << endl;
            sqlite3_free(messaggeError);
        }
        else
            cout << "Record deleted Successfully from PERSON!" << endl;
    }
 
    static int check_avail(void *data, int argc, char **argv, char **azColName)
    {
        temporaryID = argv[1];
        return 0;
    }
 
    int AssignedOrderId(string id){
        temporaryID = "-1";
        string query = "SELECT * FROM ASSIGNED_ORDER WHERE ID = \'" + id + "\';";
        sqlite3_exec(DB, query.c_str(), check_avail, NULL, NULL);
        return stoi(temporaryID);
    }
    
    static int get_wishList(void *data, int argc, char **argv, char **azColName)
    {
        temporaryID = argv[1];
        return 0;
    }

    void deleteWishList(string id){
        string sql = "DELETE FROM WISHLIST WHERE ID = \'" + id + "\';";
        exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messaggeError);
        if (exit != SQLITE_OK)
        {
            cerr << "Error DELETE" << endl;
            sqlite3_free(messaggeError);
        }
        else
            cout << "Record deleted Successfully from PERSON!" << endl;
    }

    void insertWishList(string id, string wishlist){
        string temp = '\'' + id + "\',\'" + wishlist +'\'';
        string sql("INSERT INTO WISHLIST VALUES(" + temp + ");");;
        exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messaggeError);
        if (exit != SQLITE_OK)
        {
            cerr << "Error Insert    " << messaggeError << endl;
            sqlite3_free(messaggeError);
        }
        else
            cout << "Record inserted Successfully!" << endl;
    }

    void addToWishList(string id, string name){
        temporaryID = "";
        string query = "SELECT * FROM WISHLIST WHERE ID = \'" + id + "\';";
        sqlite3_exec(DB, query.c_str(), get_wishList, NULL, NULL);
        if(temporaryID.size()!=0)deleteWishList(id);
        temporaryID += "#"+name;
        insertWishList(id,temporaryID);
    }

    set<string> Wishlist(string id){
        temporaryID = "";
        string query = "SELECT * FROM WISHLIST WHERE ID = \'" + id + "\';";
        sqlite3_exec(DB, query.c_str(), get_wishList, NULL, NULL);
        string tmp = "";
        set<string> wishlist;
        for(auto i: temporaryID){
            if(i!='#')tmp+=i;
            else wishlist.insert(tmp),tmp = "";
        }
        if(tmp.size()!=0)wishlist.insert(tmp);
        return wishlist;
    }

    void changeWishList(string id, set<string> new_wishlist){
        deleteWishList(id);
        for(auto i: new_wishlist){
            addToWishList(id, i);
        }
    }

        void payment(vector <order> Cart, enum mode payment_mode, string contact, string id){
        auto current_clock = chrono::system_clock::now();
        time_t cur_time = std::chrono::system_clock::to_time_t(current_clock);
        string currentTime = ctime(&cur_time);
        string tempMode = "";
        int totalCost = 0;
        for(auto i: Cart){
            totalCost += i.Product.price + i.Product.deliveryCharge;
        }
        cout<<"Total Cost =  "<<totalCost<<endl;
        string cardNumber, expiry_Date, Cvv,paymentUsing = "";
        switch (payment_mode)
        {
        case cashOnDelivery:
            tempMode = "CASH ON DELIVERY";
            break;
        case onlineBanking:
            cout<<"Enter card number (****-****-****-****)"<<endl;
            do{
                cin>>cardNumber;
            }while(!isCorrectCardNumber(cardNumber));
            paymentUsing = cardNumber;
            cout<<"Enter Expiry Date (mm/yy)"<<endl;
            do{
                cin>>expiry_Date;
            }while(!isCorrectDate(expiry_Date));
            cout<<"Enter Cvv (***)"<<endl;
            do{
                cin>>Cvv;
            }while(isCorrectCvv(Cvv));
            tempMode = "ONLINE BANKING";
            break;
        case Paytm:
            cout<< "Do you wish to use your current number(Y/n): "<<contact<<endl;
            char choice;
            cin>>choice;
            if(!(choice == 'Y' || choice == 'y')){
                do{
                    cin>>contact;
                }while(isContactCorrect(contact));
            }
            paymentUsing = contact;
            tempMode = "PAYTM";
            break;
        case GooglePay:
            cout<< "Do you wish to use your current number(Y/n): "<<contact<<endl;
            char choice;
            cin>>choice;
            if(!(choice == 'Y' || choice == 'y')){
                do{
                    cin>>contact;
                }while(isContactCorrect(contact));
            }
            paymentUsing = contact;
            tempMode = "GOOGLE PAY";
            break;
        default:
            cerr<<"No such banking option"<<endl;
            break;
        }
        int orderID1 =122;
        addTransaction(id,1,totalCost,orderID1, tempMode,currentTime,paymentUsing);
        }
    void setSystemState(int cusCnt, int shpCnt , int delCnt,int prodCnt,int ordCnt )
    {
        remove("systemState");
        fstream file;
        file.open("systemState",ios::out);
        systemState madeUpState;
        madeUpState.CustomerCount=cusCnt;
        madeUpState.shopKeeperCount=shpCnt;
        madeUpState.deliveryPersonCount=delCnt;
        madeUpState.orderCount=ordCnt;
        madeUpState.productCount=prodCnt;
        file.write((char*)&madeUpState,sizeof(systemState));
        file.close();
    }

};