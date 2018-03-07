void init_connector(BATTERY* batt);
void draw_connector(BATTERY* batt);

void init_scale(BATTERY* batt);
void draw_scale(BATTERY* batt);

void init_level(BATTERY* batt);
void draw_level(BATTERY* batt);

void init_info(BATTERY* batt);
void draw_info(BATTERY* batt);

void print_info_section(BATTERY* batt, unsigned colmax, const char* section);
void update_info(BATTERY* batt);