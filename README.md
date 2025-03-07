Pour compiler (ici avec `gcc`) et exécuter le code, on pourra utiliser les commandes ci-dessous:

- `gcc -mavx2 -mfma main.c dist.c -lm -lpthread -o main`

- `./main <nb_threads> <mode>`

Deux remarques :

1. Nous avons choisi d'utiliser des `double` comme type de retour de nos fonctions afin de coller aux signatures prescrites dans l'énoncé, bien que les calculs soient eux effectués en simple précision.

2. D'autre part, pour la fonction `distPar`, nous avons choisi de ne pas utiliser une variable globale protégée par un mécanisme de "mutex". Nous avons en effet trouvé plus rapide d'utiliser un tableau stockant les résultats provisoires respectifs de chacun des threads. Cela permet de ne jamais avoir de thread mis en attente alors qu'il essaie de mettre à jour la somme globale en même temps qu'un autre. De plus, la sommation finale des résultats respectifs des threads est largement négligeable.