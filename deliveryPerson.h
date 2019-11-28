#include "User.h"

class deliverPerson : public User{
    public:
    int assignedOrderId;// -1 if none is assigned
    deliverPerson(profile Profile){
        assignUserProfile(Profile);
        assignedOrderId = systemAdmin.AssignedOrderId(Profile.id);
    }
    deliverPerson(){
        //login();  
        //assignedOrderId = systemAdmin.AssignedOrderId(userID);       //////////////TEMPORARY CHANGE
        signUp();
        systemAdmin.insert_unassigned_deliveryPerson(userID);
        assignedOrderId = -1;
        systemAdmin.assignUnassignedOrders();
    }

    bool isAvailable(){
        return assignedOrderId == -1;
    }

    void checkIfOrderIsAssigned(){
        if(assignedOrderId==-1){
            cout<<"No order is assigned" << "\n";
            return;
        }
        cout<<"You have been assigned an order" << "\n";  
        cout<<"Order ID : "<<assignedOrderId<<"\n";   
        order currentOrder=systemAdmin.extactOrderInfo(to_string(assignedOrderId));
        cout<<currentOrder.order_<<"\n";
        cout<<"Customer address : "<<systemAdmin.addressFromId(currentOrder.customerID)<<"\n";
        cout<<currentOrder.other_details<<"\n";
    }

    void getProductInfoFromId(int productID){
        product currentProduct=systemAdmin.productId_to_product[productID];
        cout<<"Product Name :"<<currentProduct.product_name<<"\n";
        cout<<"Shop name :"<<systemAdmin.nameFromId(currentProduct.shopkeeper_id)<<"\n";
        cout<<"Shop address :"<<systemAdmin.addressFromId(currentProduct.shopkeeper_id)<<"\n";
    }

    void updateStatus(){
        cout<<"Enter the expected time left in dd:hh:mm format" << "\n";
        string timeLeft;cin>>timeLeft;
        systemAdmin.updateTime(to_string(assignedOrderId),timeLeft);
        if(timeLeft=="00:00:00"){
            systemAdmin.finish_order(userID);
            assignedOrderId = -1;
            systemAdmin.insert_unassigned_deliveryPerson(userID);
        }
    }

    void assignOrder(int orderId){
        assignedOrderId = orderId;
        systemAdmin.assign_order(userID, orderId);
    }

};