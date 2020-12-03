#include "bitmap.h"

/** @brief Inicializa un mapa de bits.
 *  @param bitmap * Apuntador al descriptor de mapa de bits  
 *  @param unsigned int * Region de memoria del mapa de bits
 *  @param int total de bits
 *  @return 0 
 */
int bitmap_init(bitmap * dst, unsigned int * data, int total_slots) {

    int i;
    int slot;
    int offset;
    
    unsigned int initval = ~(0);
    
    dst->data = data;   
    dst->total_slots = total_slots;
    dst->total_entries = total_slots / BITS_PER_BITMAP_ENTRY;
    
    dst->free_slots = total_slots;
    dst->last_free = -1;
    
    /* Mark BITS_PER_BITMAP_ENTRY entries at once */
    for (i = 0; i < dst->total_entries; i++) {
        data[i] = initval;
    }   
    
    /* Mark remaining entries, 
     * if total_slots is not multiple of BITS_PER_BITMAP_ENTRY */
    slot = i * BITS_PER_BITMAP_ENTRY;   
    while (slot < total_slots) {
        offset = slot % BITS_PER_BITMAP_ENTRY;
        data[i] |= (1 << offset);
        slot++;
    }
    
    return 0;
}

/* @brief Verifica el valor de un bit en el mapa de bits
 * @param bitmap * Apuntador al descriptor del mapa de bits
 * @param int Posicion del bit a verificar
 * @return int 0 o 1, dependiendo del valor de bit
 */
int bitmap_test(bitmap * dst, int slot) {
    unsigned int entry;
    int offset;
    
    if (slot < 0 || slot >= dst->total_slots) {
        return 0;
    }
    
    entry = slot / BITS_PER_BITMAP_ENTRY;
    offset = slot % BITS_PER_BITMAP_ENTRY;
    
    return (BITMAP_TEST(dst, entry, offset));

}

/* @brief Busca y limpia un bit disponible en el mapa de bits
 * @param bitmap * Apuntador al descriptor del mapa de bits
 * @return Posicion del bit en el mapa de bits, -1 si no hay disponible
 */
int bitmap_allocate(bitmap * dst){
    unsigned int slot;  
    unsigned int entry;
    unsigned int start_entry;   
    int offset; 
    
    if (dst->free_slots == 0) {return -1;}
        
    slot = dst->last_free;  
        
    if (slot >= 0 && slot < dst->total_slots) { 
        //Start at next set slot
        entry = slot / BITS_PER_BITMAP_ENTRY;
        offset = slot % BITS_PER_BITMAP_ENTRY;
        
        if (BITMAP_TEST(dst, entry, offset)) {
            /* Allocate slot! */
            BITMAP_SET(dst, entry, offset);
            dst->free_slots--;
            return slot;
        }
    }else {
        //Start at the beginnig of the bitmap
        slot = -1;
    }
    
    /* Walk the bitmap */
    slot = (slot + 1) % dst->total_slots;
    
    entry = slot / BITS_PER_BITMAP_ENTRY;
    
    start_entry = entry;
    
    do {
        if (dst->data[entry] != 0) {
            //Found entry with at least one bit set!
            offset = 0;
            while (offset < BITS_PER_BITMAP_ENTRY) {                
                if (BITMAP_TEST(dst, entry, offset)) {                  
                    /* Allocate slot! */
                    BITMAP_CLEAR(dst, entry, offset);
                    dst->free_slots--;
                    return ((entry * BITS_PER_BITMAP_ENTRY) + offset);
                }
                offset++;
            }
        }
        //Advance to next entry
        entry = (entry + 1) % dst->total_entries;       
    }while (entry != start_entry);  
    
    return -1;  
}

/* @brief Busca y limpia una region bit en el mapa de bits
 * @param bitmap * Apuntador al descriptor del mapa de bits
 * @param int Numero de bits continuos a buscar
 * @return Posicion del primer bit de la region, -1 si no hay dispnible 
 */
int bitmap_allocate_region(bitmap * dst, int count){
    unsigned int slot;  
    unsigned int entry;
    unsigned int start_entry;   
    int offset; 
    int s;
    int result;
    
    //Check for available slots
    if (count > dst->free_slots) {
        return -1;
    }
    
    slot = dst->last_free;
    
    if (slot < 0 || slot >= dst->total_slots) { 
        //Start at the beginnig of the bitmap
        slot = 0;
    }
    
    /* Walk the bitmap */
    entry = slot / BITS_PER_BITMAP_ENTRY;   
    start_entry = entry;
    
    do {
        if (dst->data[entry] != 0) {
            //Found entry with at least one bit set!
            offset = 0;
            while (offset < BITS_PER_BITMAP_ENTRY 
                    && BITMAP_TEST(dst, entry, offset) == 0) {
                offset++;
            }
            
            slot = (entry * BITS_PER_BITMAP_ENTRY) + offset;
            
            if (slot + count < dst->total_slots) {
                result = 1;
                for (s = slot; s < slot + count; s++) {
                    entry = s / BITS_PER_BITMAP_ENTRY;
                    offset = s % BITS_PER_BITMAP_ENTRY;
                    result = result && BITMAP_TEST(dst, entry, offset);
                }
                if (result) {
                    for (s = slot; s < slot + count; s++) {
                        entry = s / BITS_PER_BITMAP_ENTRY;
                        offset = s % BITS_PER_BITMAP_ENTRY;
                        BITMAP_CLEAR(dst, entry, offset);
                    }
                    dst->free_slots -= count;
                    return slot;
                }else {
                    entry = (slot  + count) / BITS_PER_BITMAP_ENTRY;
                }
            }else {
                entry = (entry + 1) % dst->total_entries;
            }
        }else {
            //Advance to next entry
            entry = (entry + 1) % dst->total_entries;       
        }
    }while (entry != start_entry);  
    
    return -1;  
}

/* @brief Marca un bit como disponible en el mapa de bits 
 * @param bitmap * Apuntador al descriptor del mapa de bits
 * @param int Posicion del bit a liberar
 * @return 0 si exitoso, -1 si error 
 */
int bitmap_free(bitmap * dst, int slot) {
    int entry;
    int offset;
    if (slot >= 0 && slot < dst->total_slots) {
        entry = slot / BITS_PER_BITMAP_ENTRY;
        offset = slot % BITS_PER_BITMAP_ENTRY;
        //Free only if clear
        if (!BITMAP_TEST(dst, entry, offset)) {
            BITMAP_SET(dst, entry,offset);
            dst->free_slots++;
            dst->last_free = slot;
      return 0;
        }
    }
  return -1;  
}

/* @brief Marca una region como disponible en el mapa de bits 
 * @param bitmap * Apuntador al descriptor del mapa de bits
 * @param int Posicion del primer bit de la region a liberar
 * @return 0 si exitoso, -1 si error 
 */
int bitmap_free_region(bitmap * dst, int slot, int count) {
    int s;
    int entry;
    int offset;
    int last_free = -1;
    if (slot >= 0 && slot + count < dst->total_slots) {
        for (s = slot; s < slot + count; s++) {
            entry = s / BITS_PER_BITMAP_ENTRY;
            offset = s % BITS_PER_BITMAP_ENTRY;
            if (!BITMAP_TEST(dst, entry, offset)) {
                BITMAP_SET(dst, entry,offset);
                dst->free_slots++;
                if (last_free == -1) {
                    last_free = s;
                }
            }
        }
        dst->last_free = last_free;
    return 0;
    }
  return -1;
}

