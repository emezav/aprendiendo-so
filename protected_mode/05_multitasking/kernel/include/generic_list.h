/**
 * @file
 * @ingroup user_code
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 *
 * @brief Contiene el macro para definir una lista de un tipo especifico,
 * con el fin de ser instanciado con diferentes tipos de datos.
 */

#ifndef GENERIC_LIST_H_
#define GENERIC_LIST_H_

#define DEFINE_GENERIC_LIST_LINKS(list_name)                                   \
	void * next_##list_name; /* Referencia al siguiente elemento */            \
	void * prev_##list_name; /* Referencia al anterior elemento */

/**  @brief Macro par generar los tipos de datos de una lista y los
 * prototipos de las funciones */
#define DEFINE_GENERIC_LIST_TYPE(element_type, list_name)                      \
typedef struct typelist_##list_name {                                          \
       element_type *head;                                                     \
       element_type *tail;                                                     \
       int count;                                                              \
}list_##list_name;                                                             \
                                                                               \
void init_list_##list_name(list_##list_name * l);                              \
                                                                               \
element_type * push_front_##list_name(list_##list_name * l,                    \
                           element_type *  n);                                 \
                                                                               \
element_type * push_back_##list_name(list_##list_name * l,                     \
                          element_type * n);                                   \
                                                                               \
element_type * insert_ordered_##list_name(list_##list_name * l,                \
                          element_type * n);                                   \
element_type * remove_##list_name(list_##list_name * l,                        \
                                element_type * n );                            \
element_type * front_##list_name(list_##list_name * l);                        \
                                                                               \
element_type * back_##list_name(list_##list_name * l);                         \
                                                                               \
element_type * pop_front_##list_name(list_##list_name * l);                    \
                                                                               \
element_type * pop_back_##list_name(list_##list_name * l);                     \
element_type * find_##list_name(list_##list_name * l,  void * value);

/**  @brief Implementación de prototipos de las funciones */
#define IMPLEMENT_GENERIC_LIST_TYPE(element_type, list_name)                   \
void init_list_##list_name(list_##list_name * l) {                             \
     l->head = 0;						                                       \
     l->tail = 0;						                                       \
     l->count = 0;						                                       \
}                                                                              \
                                                                               \
element_type *                                                                 \
       push_front_##list_name(list_##list_name * l,                            \
                           element_type *  n) {                                \
       if (l ==0) return 0;                                                    \
                                                                               \
       n->next_##list_name = 0;                                                \
       n->prev_##list_name = 0;                                                \
       if (l->head == 0) { /*Primer elemento en la lista  */                   \
          l->head = n;                                          		       \
          l->tail = n;                                          		       \
          l->count = 0;                                                        \
       }else {                                                                 \
          n->next_##list_name = l->head;                                   	   \
          l->head->prev_##list_name = n;                                       \
          l->head = n;                                          		       \
       }                                                                       \
                                                                               \
       l->count++;                                                             \
                                                                               \
       return n;                                                               \
}                                                                              \
                                                                               \
element_type *                                                                 \
       push_back_##list_name(list_##list_name * l,                             \
                          element_type * n) {                                  \
       if (l ==0) return 0;                                                    \
       n->next_##list_name = 0;                                                \
       n->prev_##list_name = 0;                                                \
                                                                               \
       if (l->tail == 0) { /*Primer elemento en la lista */                    \
          l->head = n;                                                         \
          l->tail = n;                                                         \
          l->count = 0;                                                        \
       }else {                                                                 \
          n->prev_##list_name = l->tail;                                       \
          l->tail->next_##list_name = n;                                       \
          l->tail = n;                                                         \
       }                                                                       \
       l->count++;                                                             \
                                                                               \
       return n;                                                               \
}                                                                              \
                                                                               \
element_type * insert_ordered_##list_name(                                     \
                               list_##list_name * l,                           \
                          element_type * n) {                                  \
                                                                               \
    element_type * aux, *ant;                                                  \
    if (l==0) { return 0;}                                                     \
                                                                               \
    n->next_##list_name = 0;                                                   \
    n->prev_##list_name = 0;                                                   \
    if (l->head == 0) { /*Primer elemento en la lista  */                      \
          l->head = n;                                                         \
          l->tail = n;                                                         \
          l->count = 0;                                                        \
    }else {                                                                    \
		  /* Primero verificar en el extremo de la lista */					   \
		  if (compare_##element_type(l->tail, n) > 0) {						   \
			  ant = l->tail;												   \
			  aux = 0;														   \
		  }else {															   \
			  ant = 0;                                                         \
			  aux = l->head;                                                   \
			  while (aux != 0 && compare_##element_type(aux, n) > 0) {         \
				  ant = aux;                                                   \
				  aux = aux->next_##list_name;                                 \
			  }                                                                \
		  }																	   \
                                                                               \
          n->prev_##list_name = ant;                        				   \
                                                                               \
          if (ant == 0) { /*Primer nodo de la lista  */                        \
             n->next_##list_name = l->head;                               	   \
             l->head->prev_##list_name = n;                                    \
             l->head = n;                                                      \
          }else {                                                              \
              ant->next_##list_name = n;                                   	   \
              n->next_##list_name = aux;                                   	   \
              if (aux == 0) { /* Final de la lista */                          \
                 l->tail = n;                                                  \
              }else {                                                          \
                    aux->prev_##list_name = n;                                 \
              }                                                                \
          }                                                                    \
                                                                               \
    }                                                                          \
    l->count++;                                                                \
    return n;                                                                  \
}                                                                              \
element_type *                                                                 \
      remove_##list_name(list_##list_name * l,                                 \
                                element_type * n ) {                           \
       element_type *ant, *aux;                                                \
                                                                               \
       if (l ==0) return 0;                                                    \
                                                                               \
       if (n == 0) return 0;                                                   \
                                                                               \
       ant = n->prev_##list_name;                             				   \
       aux = n->next_##list_name;                              				   \
                                                                               \
       /* Desconectar el nodo  de sus vecinos */                               \
       n->prev_##list_name = 0;                                                \
       n->next_##list_name = 0;                                                \
                                                                               \
       /* Conectar los nodos anterior y siguiente al nodo a eliminar*/         \
       if (ant != 0) {                                                         \
               ant->next_##list_name = aux;                        			   \
       }                                                                       \
                                                                               \
       if (aux != 0) {                                                         \
               aux->prev_##list_name = ant;                        			    \
       }                                                                       \
                                                                               \
       /* El nodo a remover era la cabeza? */                                  \
       if (n == l->head) {                                                     \
             l->head = aux; /* El siguiente es la nueva cabeza */              \
       }                                                                       \
                                                                               \
       /* El nodo a remover era la cola? */                                    \
       if (n == l->tail) {                                                     \
             l->tail = ant; /* El anterior es la nueva cola */                 \
       }                                                                       \
                                                                               \
        /* Un solo elemento? */                                                \
       if (l->tail == 0 || l->head == 0) { /* Un solo elemento? */             \
          l->head = 0;                                                         \
          l->tail = 0;                                                         \
       }                                                                       \
                                                                               \
       l->count--;                                                             \
                                                                               \
       return n;                                                               \
}                                                                              \
                                                                               \
element_type *                                                                 \
       front_##list_name(list_##list_name * l) {                               \
       if (l == 0) {return 0;}                                                 \
       if (l->head == 0) {return 0;}                                           \
       return l->head;                                                         \
}                                                                              \
                                                                               \
element_type *                                                                 \
       back_##list_name(list_##list_name * l) {                                \
       if (l ==0) return 0;                                                    \
             return l->tail;                                                   \
}                                                                              \
                                                                               \
element_type *                                                                 \
       pop_front_##list_name(list_##list_name * l) {                           \
       element_type *ret;                                                      \
                                                                               \
       if (l ==0) return 0;                                                    \
       /*printf("Count of nodes before pop_front: %d\n", l->count); */         \
       ret = l->head;                                                          \
       if (l->head == 0) {return ret;}                                         \
                                                                               \
       l->head = l->head->next_##list_name;                                    \
                                                                               \
       if (l->head == 0) {                                                     \
          l->tail = 0;                                                         \
       }else {                                                                 \
          l->head->prev_##list_name = 0;                                       \
       }                                                                       \
       l->count--;                                                             \
                                                                               \
       return ret;                                                             \
}                                                                              \
                                                                               \
element_type *                                                                 \
       pop_back_##list_name(list_##list_name * l) {                            \
       element_type *ret;                                                      \
                                                                               \
       if (l ==0) return 0;                                                    \
       ret = l->tail;                                                          \
                                                                               \
       if (l->tail == 0) {return ret;}                                         \
                                                                               \
       l->tail = l->tail->prev_##list_name;                                    \
                                                                               \
       if (l->tail == 0) { /* Un solo elemento? */                             \
          l->head = 0;                                                         \
       }else {                                                                 \
          l->tail->next_##list_name = 0;                                       \
       }                                                                       \
                                                                               \
       l->count--;                                                             \
                                                                               \
       return ret;                                                             \
}                                                                              \
element_type * find_##list_name(list_##list_name * l,  void * value) {         \
      element_type * h, *t;                                                    \
                                                                               \
     if (l ==0) return 0;                                                      \
                                                                               \
     h = l->head;                                                              \
     t = l->tail;                                                              \
                                                                               \
     while (h != 0 && t != 0) {                                                \
         if (equals_##element_type(h, value) == 0) {return h; }                \
         if (equals_##element_type(h, value) == 0) {return t;}                 \
                                                                               \
         if (h == t) {return 0;}                                               \
                                                                               \
         h = h->next_##list_name;                                              \
         t = t->prev_##list_name;                                              \
     }                                                                         \
     return 0;                                                                 \
}
#endif /* GENERIC_LIST_H_ */
