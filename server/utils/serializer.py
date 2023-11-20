import datetime

def make_serializable_datetimes(obj_arr):
    for idx, obj in enumerate(obj_arr):
        start_dt = obj['usage_start']
        end_dt = obj['usage_end']
        
        start_str = start_dt.strftime("%Y-%m-%d %H:%M:%S")
        end_str = end_dt.strftime("%Y-%m-%d %H:%M:%S")
        
        obj_arr[idx]['usage_start'] = start_str
        obj_arr[idx]['usage_end'] = end_str
    
    return obj_arr