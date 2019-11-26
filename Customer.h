#include "User.h"
#include "stringMatching.h"

//isblacklisted to be updated in admin.h

class customer : public User
{
public:
    vector<pair<product, int>> cart;
    customer(profile userProfile)
    {
        name = userProfile.name;
        surname = userProfile.surname;
        emailID = userProfile.email;
        contact = userProfile.contact;
        username = userProfile.username;
        password = userProfile.password;
        userID = userProfile.id;
        address = userProfile.address;
        userType=Customer;
    }

    /////////////TEMPORARY CHANGE/////////////
    customer(){
        login();
    }

    static bool sortByRating(product product1, product product2)
    {
        return (product1.rating > product2.rating);
    }

    static bool sortByIncreasingPrice(product product1, product product2)
    {
        return (product1.price < product2.price);
    }

    static bool sortByDecreasingPrice(product product1, product product2)
    {
        return (product1.price > product2.price);
    }

    void search()
    {
        printHeader();
        cout<<endl;
        cout << printtabs(8);
        cout << fggreen;
        cout << "Enter the product you want to look-up";
        PRINTBLUE;
        cout << " ";
        cout << fggreen;
        string toBeSearched;
        cin >> toBeSearched;
        cout<<endl;
        cout<<fggreen<<printtabs(8)<<"     ";
        cout << "Do you want a filter for your search results? Y/n"
             << endl;
        printInputField();
        char response;
        cin >> response;
        int input;
        if (response == 'y' || response == 'Y')
        {
            cout<<fgblue<<printtabs(8)<<"    ";
            cout << "Choose your filter of products"
                 << endl;
            cout<<endl;
            printOption(8,0,"Decreasing order of rating",1);
            printOption(8,0,"Increasing order of Price ",2);
            printOption(8,0,"Decrasing order of Price ",3);
            printInputField();
            cin >> input;
        }
        vector<product> matches;
        bool isFullMatchPossible = 0;
        for (auto currentProduct : systemAdmin.global_inven_map)
        {
            if (toBeSearched == currentProduct.first)
            {
                isFullMatchPossible = 1;
                break;
            }
        }
        for (auto currentProduct : systemAdmin.global_inven_map)
        {
            bool toAdd = 0;
            if (isFullMatchPossible)
            {
                if (toBeSearched == currentProduct.first)
                    toAdd = 1;
            }
            else
                toAdd = isMatch(currentProduct.first, toBeSearched);
            if (toAdd)
            {
                for (auto ids : currentProduct.second)
                {
                    if (systemAdmin.productId_to_product[ids].count)
                        matches.push_back(systemAdmin.productId_to_product[ids]);
                }
            }
        }
        if (response != 'y' && response != 'Y')
            ;
        else if (input == 1)
            sort(matches.begin(), matches.end(), sortByRating);
        else if (input == 2)
            sort(matches.begin(), matches.end(), sortByIncreasingPrice);
        else
            sort(matches.begin(), matches.end(), sortByDecreasingPrice);
        
        int productMatchCounter=1;
        for (auto currentProduct : matches)
        {
            cout<<fgblue<<printtabs(8)<<"PRODUCT NUMBER :"<<fgred<<productMatchCounter;
            cout<<endl;
            cout << "Product name : " << string(currentProduct.product_name) << "\n";
            cout << "Product ID : " << currentProduct.product_id << "\n";
            cout << "Shopkeeper : " << systemAdmin.nameFromId(currentProduct.shopkeeper_id) << "\n";
            cout << "Rating : " << currentProduct.rating << "\n";
            cout << "Quantity : " << currentProduct.count << "\n";
            cout << "Price : " << currentProduct.price << "\n";
            cout << "Delivery Charges : " << currentProduct.deliveryCharge << "\n";
            cout << "\n";
        }
    }

    void addToCart()
    {
        cout << "Enter the ID of product you wish to add to cart "
             << "\n";
        int productID;
        cin >> productID;
        cout << "Enter the quantity of purchase"
             << "\n";
        int quantity;
        cin >> quantity;
        cart.push_back({systemAdmin.productId_to_product[productID], quantity});
    }

    void removeFromCart()
    {
        cout << "Enter the ID of product you want to remove from cart"
             << "\n";
        int productID;
        cin >> productID;
        for (int i = 0; i < cart.size() - 1; ++i)
        {
            if (cart[i].first.product_id == productID)
                swap(cart[i], cart.back());
        }
        cart.pop_back();
    }

    void displayCart()
    {
        if (cart.empty())
        {
            cout << "Your cart is empty"
                 << "\n";
            return;
        }
        for (auto y : cart)
        {
            cout << "Product name : " << y.first.product_name << "\n";
            cout << "Product ID : " << y.first.product_id << "\n";
            cout << "Price : " << y.first.price << "\n";
            cout << "Delivery Charges : " << y.first.deliveryCharge << "\n";
            cout << "Quantity you have added to cart : " << y.second << "\n";
            cout << "\n";
        }
    }

    void cashInTheCart()
    {
        cout << "Choose your payment mode"
             << "\n";
        cout << "Press 1 for cash on delivery, 2 for online banking, 3 for paytm, 4 for gpay"
             << "\n";
        int response;
        cin >> response;
        enum mode paymentMode;
        if (response == 1)
            paymentMode = cashOnDelivery;
        else if (response == 2)
            paymentMode = onlineBanking;
        else if (response == 3)
            paymentMode = Paytm;
        else if (response == 4)
            paymentMode = GooglePay;
        else
            cout << "Invalid banking option" << endl;
        set<int> toBeRemoved;
        for (int i = 0; i < cart.size(); ++i)
        {
            auto y = cart[i];
            int availableQuantity = systemAdmin.productId_to_product[y.first.product_id].count;
            if (y.second > availableQuantity)
            {
                cout << "Only " << availableQuantity << "are available"
                     << "\n";
                cout << "Product Name : " << y.first.product_name << "\n";
                cout << "Product ID : " << y.first.product_id << "\n";
                cout << "Press 1 if you want to remove this from cart"
                     << "\n";
                cout << "Press 2 if you want to order the available quantity"
                     << "\n";
                int response;
                cin >> response;
                if (response == 1)
                    toBeRemoved.insert(y.first.product_id);
                else
                    cart[i].second = availableQuantity;
            }
        }
        for (int i = 0; i < cart.size(); ++i)
        {
            auto y = cart[i];
            if (toBeRemoved.find(y.first.product_id) == toBeRemoved.end())
                continue;
            swap(cart[i], cart.back());
            cart.pop_back();
        }

        systemAdmin.payment(cart, paymentMode, contact, userID);
        cart.clear();
    }

    void addToWishlist()
    {
        cout << "Enter the name of product you want to add to your wishlist"
             << "\n";
        string productName;
        cin >> productName;
        if (systemAdmin.global_inven_map.find(productName) == systemAdmin.global_inven_map.end())
        {
            cout << "This product is not available"
                 << "\n";
            return;
        }
        systemAdmin.addToWishList(userID, productName);
    }

    void displayWishlist()
    {
        set<string> tempWishlist = systemAdmin.returnWishlist(userID);
        if (tempWishlist.empty())
        {
            cout << "Your wishlist is empty"
                 << "\n";
            return;
        }
        for (auto y : tempWishlist)
            cout << y << " ";
        cout << endl;
    }

    void removeFromWishlist()
    {
        cout << "Enter the name of product you want to remove from wishlist"
             << "\n";
        string toRemove;
        cin >> toRemove;
        set<string> tempWishlist = systemAdmin.returnWishlist(userID);
        if (tempWishlist.find(toRemove) == tempWishlist.end())
        {
            cout << "This item does not belong to your wishlist"
                 << "\n";
            return;
        }
        tempWishlist.erase(tempWishlist.find(toRemove));
        systemAdmin.changeWishList(userID, tempWishlist);
    }

    void displayYourOrders(){
        vector<int> orders=systemAdmin.orderIdsofCustomer(userID);
        for(auto order_id:orders){
            order currentOrder=systemAdmin.extactOrderInfo(to_string(order_id));
            cout<<"Order ID :"<<order_id<<"\n";
            cout<<currentOrder.order_<<"\n";
            if(currentOrder.remainingTime!="00:00:00")cout<<currentOrder.remainingTime<<"\n";
            cout<<currentOrder.other_details<<"\n";
        }
    }

    void checkStatus(){
        cout<<"Enter the ID of order you want to check for status" << "\n";
        int orderID;cin>>orderID;
        cout<<"time left : "<<systemAdmin.get_orderStatus(to_string(orderID));
    }
};