/*
 * Taken from Simplemon, by Emiel Kollof
 * 
 * License is BeerWare (c) Poul-Henning Kamp. In other words:
 * 
 * Emiel Kollof wrote this program. You may do almost anything with this source
 * that you like, except claiming that you wrote it yourself. If you think
 * this thing is useful, you may reward me by buying me a beer if we ever
 * should meet one day.
 */

#ifndef lint
static const char copyright[] =
"Copyright (c) 2003 Emiel Kollof <coolvibe@hackerheaven.org>";
#endif

#ifndef lint
static const char rcsid[] =
"$Id: list.c,v 1.3 2003/09/03 17:53:44 coolvibe Exp $";
#endif

#include "sysalert.h"

/*
 * REALLY simple and inefficient linked list stuff. I'm not proud of this,
 * but at least it works and it is somewhat clean-ish
 */

node           *
mylist_malloc()
{

	/*
	 * mallocs a node the right size. Don't forget to free it when it
	 * gets deleted. This saves me some typing :)
	 */

	return (node *) malloc(sizeof(node));
}

void
mylist_store(node * new, node ** start,
	     node ** last)
{
	/*
	 * this function sorts the entries in the linked list prior to
	 * inserting them. Does some icky inefficient whole list traversal,
	 * but at least we don't get a jumble of stuff
	 * 
	 * Don't forget to put NULLs in start and last when you start a new
	 * list.
	 */

	node           *old, *p;

	if (*last == NULL) {
		/* we got a new list */
		new->next = NULL;
		new->prev = NULL;
		*last = new;
		*start = new;
		return;
	}
	/* check if the entry is already in the list */
	p = (node *) mylist_find(*start, new->data);
	if (p)
		return;

	p = *start;

	old = NULL;
	while (p) {
		if (strcmp(p->data, new->data) < 0) {
			old = p;
			p = p->next;
		} else {
			if (p->prev) {
				/* insert somewhere in the middle */
				p->prev->next = new;
				new->next = p;
				new->prev = p->prev;
				p->prev = new;
				return;
			}
			/* new first item */
			new->next = p;
			new->prev = NULL;
			p->prev = new;
			*start = new;
			return;
		}
	}
	/* tack on the end */
	old->next = new;
	new->next = NULL;
	new->prev = old;
	*last = new;
	return;
}

void
mylist_destroy(node * start)
{
	/*
	 * This function deallocates all the nodes in the linked list so we
	 * won't get any nasty memory leaks
	 */

	node           *hp;

	while (start) {
		hp = start->next;
		free(hp);
		start = hp;
	}
	return;
}

void
mylist_print(node * start)
{
	/*
	 * This is just a little function that prints what's in the double
	 * linked list. Not really intended for the program, but handy for
	 * getting the list stuff right. It just prints what's in there
	 */

	node           *tstart;

	tstart = start;

	while (tstart) {
		printf("Data :\t%s\n", tstart->data);
		tstart = tstart->next;
	}

	return;
}

node           *
mylist_find(node * start, char *entry)
{

	/*
	 * This function traverses the list and tries to find an entry based
	 * on the hostname
	 */

	node           *p;

	p = start;
	while (p) {
		if (!strcmp(entry, p->data))
			return p;	/* match */
		p = p->next;
	}
	return NULL;		/* no match */
}

int
mylist_delete(char *entry, node ** start, node ** last)
{
	/*
	 * finds a node, and then remaps the prev and next pointers so it's
	 * not a logical node in the list in the more. It gets freed
	 * afterwards
	 */

	node           *hp;

	hp = mylist_find(*start, entry);
	if (hp) {
		if (*start == hp) {
			*start = hp->next;
			if (*start)
				(*start)->prev = NULL;
			else
				*last = NULL;
		} else {
			hp->prev->next = hp->next;
			if (hp != *last)
				hp->next->prev = hp->prev;
			else
				*last = hp->prev;
		}
		free(hp);
		return (0);
	} else
		return (-1);
}

int
mylist_save(node * start, char *filename)
{
	/*
	 * Dumps the linked list to a file. I'll probably never use this, but
	 * it just might come in handy
	 */

	node           *hp;
	FILE           *fp;

	fp = fopen(filename, "wb");
	if (!fp) {
		perror("fopen");
		return -1;
	}
	hp = start;
	while (hp) {
		if (fwrite(hp, sizeof(node), 1, fp) < 1) {
			perror("mylist_save: fwrite");
		}
		hp = hp->next;	/* next item */
	}
	fclose(fp);
	return (0);
}

int
mylist_load(node * start, node * last, char *filename)
{
	/* Loads the list from a file. Same purpose as mylist_save. */

	node           *hp = NULL;
	FILE           *fp;

	fp = fopen(filename, "rb");
	if (!fp) {
		perror("fopen");
		return -1;
	}
	mylist_destroy(start);	/* deallocate old list */

	start = last = NULL;	/* reset start and last pointers */

	while (!feof(fp)) {
		hp = mylist_malloc();
		if (fread(hp, sizeof(node), 1, fp) != 1)
			break;
		mylist_store(hp, &start, &last);
	}
	fclose(fp);

	return (0);
}

void 
mylist_additem(node * list, char *data, node ** start, node ** last)
{
	list = mylist_malloc();
	if (asprintf(&(list->data), "%s", data) < 0) {
		perror("mylist_additem: asprintf");
	}
	mylist_store(list, start, last);
	return;
}
