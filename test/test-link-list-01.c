#include <linklist.h>

int main()
{
    struct link_list_t list;
    struct link_node_t node[5];
    int i;

    link_list_init (&list);
    link_list_attach_to_tail (&list, node+0);

    assert (list.head == list.tail);
    assert (list.head == node + 0);
    assert (node[0].prev == node[0].next);
    assert (node[0].next == node + 0);

    link_list_attach_to_tail (&list, node+1);
    link_list_attach_to_head (&list, node+2);

    assert (node[0].next = node + 1);
    assert (node[0].prev = node + 2);
    assert (node[1].prev == node + 0);
    assert (node[2].next == node + 0);
    assert (node[1].next == node + 1);
    assert (node[2].prev == node + 2);

    link_list_dettach (&list, node + 0);
    link_list_dettach (&list, node + 1);
    link_list_dettach (&list, node + 2);

    assert (link_node_is_dettached (node+0));
    assert (link_node_is_dettached (node+1));
    assert (link_node_is_dettached (node+2));
    assert (link_list_is_empty (&list));

    for (i = 0; i < 5; i++)
        link_list_attach_to_tail (&list, node+i);
    link_list_dettach (&list, node+0);
    link_list_dettach (&list, node+2);
    link_list_dettach (&list, node+4);

    assert (link_node_is_dettached (node+0));
    assert (link_node_is_dettached (node+2));
    assert (link_node_is_dettached (node+4));
    assert (list.head == node + 1);
    assert (list.tail == node + 3);
    assert (node[1].prev == node + 1);
    assert (node[3].next == node + 3);
    assert (node[1].next == node + 3);
    assert (node[3].prev == node + 1);
    return 0;
}
