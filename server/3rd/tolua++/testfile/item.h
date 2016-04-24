//tolua_begin

class User {
public:
    int hello(char *str);

    int uid;
};

namespace Item {
    int add_item(int itemid, int itemcount);
};
//tolua_end
