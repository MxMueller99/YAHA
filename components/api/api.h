#ifndef API_H
#define API_H

void get_current_weather_data(char *buffer, int buffer_size, char *temp_buffer, int temp_buffer_size, char *main_buffer, int main_buffer_size);
void get_weather_forecast(char *buffer, int buffer_size);

#endif