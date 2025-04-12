#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_LEN 256
#define BUFFER_SIZE 1024
#define TABLE_SIZE 2048


typedef struct lot {
    int expiry;
    int amount;
    struct lot* next;
} lot;

typedef struct head_of_lot {
    char *name;
    lot* lots;
    struct head_of_lot* next;
    int total_quantity;
} head_of_lot;

typedef struct Ingredient {
    int amount;
    struct Ingredient *next_ingredient;
    struct head_of_lot *warehouse_ingrediente;
    //puntatore a magazzino
} Ingredient;


typedef struct Recipe {
    char *name;
    Ingredient *ingredients;
    int weight;
    struct Recipe *next_recipe;
} Recipe;

typedef struct Order {
    int amount;
    int order_time;
    Recipe* recipe;
    struct Order* next_order;
} Order;

Recipe* recipetable[TABLE_SIZE] = {NULL};

head_of_lot* warehousetable[TABLE_SIZE] = {NULL};

void remove_lots( Recipe* rem_rec, int multiplier );
void insert_ready(  Order* order, Order** ready_list_head, Order** ready_list_tail );

//had to make my own hash function 
//not the best but it works and is quick enough
int hash(char *name) {
    int hash_value = 3299;
    while(*name) {
        hash_value += *name++;
    }
    return abs( hash_value % TABLE_SIZE ) ;
}

Recipe *search_recipe( char *name){
    int index;
    index = hash(name);
    Recipe* temp;
    if( recipetable[index]==NULL){
        return NULL;
    } else{
        temp=recipetable[index];
        if( strcmp( temp->name, name )== 0 ){
            return temp;
        } else {

            while(temp != NULL && strcmp(temp->name, name) != 0) {
                temp = temp-> next_recipe;
            }

            return temp;
        }
    }
}

Recipe* add_recipe( char *name ){
    int index;
    Recipe *temp;

    if( search_recipe(name) != NULL )
        return NULL;

    temp = (Recipe*) malloc( sizeof(Recipe) ); //casting before malloc
    temp->name = (char*) malloc(strlen(name)+1);
    temp->ingredients=NULL;
    temp->next_recipe =NULL;
    temp->weight =0;
    strcpy(temp->name, name);

    index = hash( name );

    temp->next_recipe = recipetable[index];
    recipetable[index] = temp;

    return temp;
}

void remove_recipe(char *name, Order* order_list_head, Order*  ready_list_head ){
    int index;
    Recipe *temp;
    Recipe *prev;
    index = hash( name );
    temp = recipetable[index];
    prev = recipetable[index];
    Order* var = order_list_head;

    while( var != NULL) {
        if( strcmp( var->recipe->name, name) == 0 ) {
            printf("ordini in sospeso\n");
            return;
        }
        var = var->next_order;
    }

    var = ready_list_head;
    while( var != NULL) {
        if( strcmp( var->recipe->name, name) == 0 ) {
            printf("ordini in sospeso\n");
            return;
        }
        var = var->next_order;
    }

    if(temp == NULL){
        printf("non presente\n");
        return;
    }

    if(strcmp( temp->name, name )== 0 ){
        recipetable[index] = temp->next_recipe;
    } else{
        temp= temp->next_recipe;
        while(temp != NULL && strcmp(temp->name, name) != 0){
            prev = temp;
            temp=temp->next_recipe;
        }
        if(temp == NULL){
            printf("non presente\n");
        } else{
            prev->next_recipe = temp->next_recipe;
        }
    }

    // e non è usato da ordini in sospeso
    if(temp != NULL){
        Ingredient *meh;
        Ingredient *lol;
        free(temp->name);
        meh= temp->ingredients;
        while(meh != NULL) {
            lol = meh;
            meh = meh->next_ingredient;
            free(lol);
        }
        free(temp);

        printf("rimossa\n");
    }
}

//###############################################################################

head_of_lot *search_warehouse( char *name){
    int index;
    index = hash(name);
    head_of_lot * temp;
    if( warehousetable[index]==NULL){
        return NULL;
    } else{
        temp=warehousetable[index];
        if( strcmp( temp->name, name )== 0 ){
            return temp;
        } else {

            while(temp != NULL && strcmp(temp->name, name) != 0) {
                temp = temp->next;
            }

            return temp;
        }
    }
}
head_of_lot* add_warehouse( char *name ){
    int index;
    head_of_lot *temp;

    temp = search_warehouse(name);
    if( temp != NULL ){
        return temp;
    }

    temp = (head_of_lot*) malloc( sizeof(head_of_lot) ); //casting prima di malloc
    temp->name = (char*) malloc(strlen(name)+1);
    temp->lots=NULL;
    temp->next =NULL;
    temp->total_quantity = 0;
    strcpy(temp->name, name);

    index = hash( name );

    temp->next = warehousetable[index];
    warehousetable[index] = temp;

    return temp;
}

//manca 1 condizione ma questa funzione non servirà per il test
/*
void warehouse_remove(char *name){
    //chiamo solo se coda lotti vuota
    int index;
    head_of_lot *temp;
    head_of_lot *prev;
    index = warehouse_hash_function( name );
    temp = warehousetable[index];
    prev = warehousetable[index];

    if( temp !=NULL && strcmp( temp->name, name ) == 0 ){
        warehousetable[index] = temp->next;
    } else{
        // porto temp = nel while??
        temp= temp->next;
        while(temp != NULL && strcmp(temp->name, name) != 0){
            prev = temp;
            temp=temp->next;
        }
        if(temp == NULL){
            return;
        } else{
            //controlla che non sia usato da ordini in sospeso
            prev->next = temp->next;
        }
    }
    free(temp->name);
    free(temp);
}

 */

//##################################################################################

void add_lot( head_of_lot* head, lot* lot_to_add, int current_time) {

    lot *prev;
    lot *temp;
    head->total_quantity = head->total_quantity + lot_to_add->amount;

    temp = head->lots;
    while (temp != NULL && temp->expiry <= current_time) {
        head->lots = temp->next;
        head->total_quantity = head->total_quantity - temp->amount;
        free(temp);
        temp = head->lots;
    }

    if(head->lots == NULL){
        lot_to_add->next = NULL;
        head->lots = lot_to_add;
    }
    else if(head->lots->expiry >= lot_to_add->expiry){
        lot_to_add->next = head->lots;
        head->lots = lot_to_add;
    }
    else{
        temp = head->lots;
        prev = head->lots;

        while( temp !=NULL && temp->expiry < lot_to_add->expiry){
            prev = temp;
            temp = temp->next;
        }

        prev->next = lot_to_add;
        lot_to_add->next = temp;
    }
}

int check_can_cook( Recipe* recipe, int multiplier, int current_time){
    Ingredient* ingr;
    head_of_lot * searched;
    lot* temp_lot;

    // Il secondo controlla se c'è la quantità necessaria
    ingr = recipe->ingredients;

    //scorro ingredienti
    while(ingr != NULL ) {
        searched = ingr->warehouse_ingrediente;
        temp_lot = ingr->warehouse_ingrediente->lots;
        // se c'è solo l'head ma non ci sono lotti

        if (temp_lot == NULL) {
            //warehouse_remove(ingr->warehouse_ingrediente->name );
            return 0;
        }
        // while per togliere i lotti scaduti
        while( temp_lot != NULL && temp_lot->expiry <= current_time ) {
            searched->lots = temp_lot->next;
            searched->total_quantity = searched->total_quantity - temp_lot->amount;
            free(temp_lot);
            temp_lot = searched->lots;
        }

        if ( searched->total_quantity < multiplier * ingr->amount ) {
            return 0;
        }

        ingr = ingr->next_ingredient;
    }

    return 1;
}

void add_order( char* name, int amount, int time, Order** order_list_head, Order** order_list_tail, Order** ready_list_head, Order** ready_list_tail) {
    Recipe* recipe_pointer = search_recipe(name);
    if( recipe_pointer == NULL ) {
        printf("rifiutato\n");
    } else {
        // qui creo un nuovo ordine
        Order* new_order = (Order*) malloc(sizeof(Order));
        new_order->recipe = recipe_pointer;
        new_order->order_time = time;
        new_order->next_order = NULL;

        new_order->amount = amount;

        if(check_can_cook(recipe_pointer, amount, time) == 1){
            remove_lots(recipe_pointer, amount);
            insert_ready(new_order, ready_list_head, ready_list_tail);
        }else{
            if( *order_list_head != NULL ) {
                (*order_list_tail)->next_order = new_order;
                (*order_list_tail) = new_order;
            } else {
                (*order_list_head) = new_order;
                (*order_list_tail) = new_order;
            }
        }

        printf("accettato\n");
    }
}

void insert_ready(  Order* order, Order** ready_list_head, Order** ready_list_tail ) {
    Order* head = *ready_list_head;
    Order* prev = NULL;

    while(head != NULL && head->order_time < order->order_time){
        prev = head;
        head = head->next_order;
    }

    if(prev == NULL){
        order->next_order = *ready_list_head;
        *ready_list_head = order;
    } else{
        prev->next_order = order;
        order->next_order = head;
    }

    if(*ready_list_tail == NULL){
        *ready_list_tail = *ready_list_head;
    }
    else if((*ready_list_tail)->next_order != NULL){
        *ready_list_tail = (*ready_list_tail)->next_order;
    }
}

void remove_lots( Recipe* rem_rec, int multiplier ) {


    Ingredient* ingr = rem_rec->ingredients;
    lot* prev = NULL;
    head_of_lot* head_lot = ingr->warehouse_ingrediente;
    int spia = 0;

    while(ingr != NULL ) {
        int counter = 0;
        spia = 0;
        head_lot = ingr->warehouse_ingrediente;
        head_lot->total_quantity = head_lot->total_quantity - (ingr->amount*multiplier);

        while(head_lot->lots != NULL && spia == 0 ) {
            counter = counter + head_lot->lots->amount;
            if(counter < ingr->amount * multiplier ) {
                prev =  head_lot->lots;
                head_lot->lots = prev->next;
                free(prev);
            } else if (counter > ingr->amount * multiplier ) {
                head_lot->lots->amount = counter - (ingr->amount * multiplier);
                spia = 1;
            } else {
                // caso =
                prev =  head_lot->lots;
                head_lot->lots = prev->next;
                free(prev);
                spia = 1;
            }
        }

        ingr = ingr->next_ingredient;
    }
}

void lets_cook( Order** order_list_head, Order** order_list_tail, Order** ready_list_head, Order** ready_list_tail, int current_time ) {
    //scorre lista ordini dalla testa e verifica che siano cucinabili, se lo sono li cucina
    Order* temp = *order_list_head;
    Order* prev = NULL;
    while( temp != NULL) {

        if( check_can_cook( temp->recipe, temp->amount, current_time ) == 1 ) {
            remove_lots( temp->recipe, temp->amount );

            if ( temp != *order_list_head && temp != *order_list_tail ) {
                //OK
                prev->next_order = temp->next_order;
                insert_ready(temp,  ready_list_head, ready_list_tail)  ;
                temp = prev->next_order;

            } else if( temp == *order_list_head) {
                if(*order_list_head == *order_list_tail) {
                    *order_list_tail = NULL;
                    insert_ready( temp, ready_list_head, ready_list_tail );
                    *order_list_head = NULL;
                    return;
                }
                *order_list_head = (*order_list_head)->next_order;
                insert_ready( temp, ready_list_head, ready_list_tail );
                temp = *order_list_head;

            } else if( temp == *order_list_tail) {
                if(*order_list_head == *order_list_tail) {
                    order_list_head = NULL;
                }
                *order_list_tail = prev;
                prev->next_order = NULL;
                insert_ready( temp, ready_list_head, ready_list_tail );
                return;
            }
        } else {
            prev = temp;
            temp = temp->next_order;
        }
    }

}

Order* insert_sorted_order(Order* head, Order* move){
    Order* temp = head;
    Order* prev = NULL;

    if ( head == NULL){
        head = move;
        return head;
    }

    while( temp != NULL && (move->amount * move->recipe->weight  < temp->amount * temp->recipe->weight
    || (move->amount * move->recipe->weight  ==  temp->amount * temp->recipe->weight && move->order_time > temp->order_time))){
        prev = temp;
        temp = temp->next_order;
    }

    if(prev != NULL){
        prev->next_order = move;
        move->next_order = temp;
    }
    else{
        move->next_order = head;
        head = move;
    }

    return head;

}

void load_truck( Order** ready_list_head, Order** ready_list_tail, int truck_capacity ) {
    Order* temp = *ready_list_head;
    Order* move = NULL;
    Order* head = NULL;
    Order* del = NULL;
    int total_weight = 0;
    if( *ready_list_head == NULL ) {
        printf("camioncino vuoto\n");
        return;
    }

    while( temp != NULL && total_weight < truck_capacity) {
        total_weight = total_weight + (temp->recipe->weight*temp->amount);
        if (total_weight <= truck_capacity ) {
            move = temp;
            temp = temp->next_order;
            move->next_order = NULL;
            head = insert_sorted_order(head, move );
        }
        else
            break;
    }

    *ready_list_head = temp;
    if( *ready_list_head == NULL ){
        *ready_list_tail = NULL;
    }

    while( head != NULL ){
        printf( "%d %s %d\n", head->order_time, head->recipe->name, head->amount );
        del = head;
        head = head->next_order;
        free(del);
    }
}



int main(int argc, char* argv[]){
    char buff[MAX_NAME_LEN];
    int quit = 0;
    int recipe_weight = 0;
    int amount = 0;
    int expiry = 0;
    int current_time = 0;
    int truck_time = 0;
    int truck_capacity = 0;
    Recipe* recipe_to_add = NULL;
    lot* lot_to_add = NULL;
    Ingredient* ingredient_to_add = NULL;
    head_of_lot* temp = NULL;

    //liste degli ordini
    Order* order_list_head = NULL;
    Order* order_list_tail = NULL;
    Order* ready_list_head = NULL;
    Order* ready_list_tail = NULL;
    int var;


    var = scanf( "%d %d", &truck_time, &truck_capacity );
    fprintf(stderr, "%d", var);

    while(quit == 0){
        var = scanf("%s", buff);

        if (current_time % truck_time == 0 && current_time != 0) {
            load_truck( &ready_list_head, &ready_list_tail, truck_capacity );
        }

        if(buff[0] == '\0'){
            quit = 1;
        }
        else{
            if(strcmp(buff,"aggiungi_ricetta") == 0){
                var = scanf("%s", buff);
                recipe_to_add = add_recipe(buff);
                recipe_weight = 0;
                if(recipe_to_add == NULL){
                    printf("ignorato\n");
                    while(fgetc(stdin) != '\n');
                } else {
                    printf("aggiunta\n");
                    while(fgetc(stdin) != '\n'){
                        var = scanf("%s %d", buff, &amount);
                        ingredient_to_add = (Ingredient*) malloc(sizeof(Ingredient));
                        ingredient_to_add->amount = amount;
                        ingredient_to_add->warehouse_ingrediente = add_warehouse(buff);
                        recipe_weight = recipe_weight + amount;
                        ingredient_to_add->next_ingredient=recipe_to_add->ingredients;
                        recipe_to_add->ingredients = ingredient_to_add;
                    }
                    recipe_to_add->weight = recipe_weight;
                }
            }else if( strcmp(buff,"rimuovi_ricetta") == 0 ){
                var = scanf("%s", buff);
                remove_recipe( buff, order_list_head, ready_list_head );
            } else if( strcmp(buff,"rifornimento") == 0 ){
                while(fgetc(stdin) != '\n'){
                    var = scanf("%s %d %d", buff, &amount, &expiry);
                    if( expiry > current_time){
                        lot_to_add = (lot*) malloc(sizeof(lot));
                        lot_to_add->amount = amount;
                        lot_to_add->expiry = expiry;
                        temp = add_warehouse(buff);
                        add_lot(temp,lot_to_add, current_time);

                    }
                }
                printf("rifornito\n");
                //vai in cucina
                lets_cook( &order_list_head, &order_list_tail, &ready_list_head,  &ready_list_tail,  current_time );


            } else if( strcmp(buff,"ordine") == 0 ) {
                var = scanf("%s %d", buff, &amount);
                add_order( buff, amount, current_time, &order_list_head, &order_list_tail , &ready_list_head, &ready_list_tail);
            }
            else{
                fprintf(stderr, "comando errato\n");
                exit(-1);
            }
        }

        buff[0] = '\0';
        current_time++;
    }

    fprintf(stderr, "%d", var);

    return 0;
}


