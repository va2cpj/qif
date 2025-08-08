

// Define the filter structure
typedef struct {
    uint16_t filt_addr;   // address to filter 
    uint8_t  filt_type;   // TypeFilter: 0 pass; 1 block; 3 start_range; 4 end_range
    uint8_t  fct_addr;    // function address (0x00..0xFF)
} filter;

// Define the message structure received externally
typedef struct {
    uint8_t  N_filt;      // index of filter in FilterDB (0..MAX_FILTERS-1)
    uint16_t filt_addr;
    uint8_t  filt_type;
    uint8_t  fct_addr;
} rcv_filter_msg;

// Global filter database
filter FilterDB[MAX_FILTERS];

// Function to apply received filter to database
void f_rcv_filter(rcv_filter_msg rcv_filter)
{
    if (rcv_filter.N_filt < MAX_FILTERS)
    {
        FilterDB[rcv_filter.N_filt] = (filter){
            .filt_addr = rcv_filter.filt_addr,
            .filt_type = rcv_filter.filt_type,
            .fct_addr  = rcv_filter.fct_addr
        };
    }
    // Optional: else handle invalid index
}